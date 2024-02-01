#include <iostream>
#include "../../SessionLayer/SessionLayer.h"
#include "SequencerAlgoLayer.h"
#include "SequencerAlgoLayerMsg.h"
#include "../../msgTemplates.h"

using namespace std;
using namespace fbae_SequencerAlgoLayer;

bool SequencerAlgoLayer::callbackHandleMessage(std::unique_ptr<CommPeer> peer, std::string && msgString)
{
    auto msgId{ static_cast<MsgId>(msgString[0]) };
    switch (msgId)
    {
        using enum MsgId;
        //
        // Cases corresponding to messages received by sequencer
        //
        case DisconnectIntent :
        {
            auto bdi{deserializeStruct<StructDisconnectIntent>(std::move(msgString))};
            auto s{serializeStruct<StructAckDisconnectIntent>(StructAckDisconnectIntent{MsgId::AckDisconnectIntent})};
            peer->sendMsg(std::move(s));
            if (getSession()->getParam().getVerbose())
                cout << "\tSequencerAlgoLayer / Sequencer : Broadcaster #" << static_cast<uint32_t>(bdi.senderRank) << " announces it will disconnect.\n";
            break;
        }
        case RankInfo :
        {
            auto bri{deserializeStruct<StructRankInfo>(std::move(msgString))};
            if (getSession()->getParam().getVerbose())
                cout << "\tSequencerAlgoLayer / Sequencer : Broadcaster #" << static_cast<uint32_t>(bri.senderRank) << " is connected to sequencer.\n";
            if (++nbConnectedBroadcasters == getBroadcasters().size())
            {
                if (getSession()->getParam().getVerbose())
                    cout << "\tSequencerAlgoLayer / Sequencer : All broadcasters are connected: Broadcast AllBroadcastersConnected\n";
                auto s{serializeStruct<StructAllBroadcastersConnected >(StructAllBroadcastersConnected{MsgId::AllBroadcastersConnected})};
                getSession()->getCommLayer()->broadcastMsg(std::move(s));
            }
            break;
        }
        case MessageToBroadcast :
        {
            auto msgToBroadcast{deserializeStruct<StructMessageToBroadcast>(std::move(msgString))};
            auto s {serializeStruct<StructBroadcastMessage>(StructBroadcastMessage{MsgId::BroadcastMessage,
                                                                                   msgToBroadcast.senderRank,
                                                                                   msgToBroadcast.sessionMsg})};
            getSession()->getCommLayer()->broadcastMsg(std::move(s));
            break;
        }
        //
        // Cases corresponding to messages received by a broadcaster
        //
        case AckDisconnectIntent :
            peer->disconnect();
            break;
        case AllBroadcastersConnected :
            getSession()->callbackInitDone();
            break;
        case BroadcastMessage :
        {
            auto sbm {deserializeStruct<StructBroadcastMessage>(std::move(msgString))};
            getSession()->callbackDeliver(sbm.senderRank,std::move(sbm.sessionMsg));
            break;
        }
        default:
        {
            cerr << "ERROR\tSequencerAlgoLayer: Unexpected msgId (" << static_cast<int>(msgId) << ")\n";
            exit(EXIT_FAILURE);
        }
    }
    return msgId == MsgId::AckDisconnectIntent;
}

bool SequencerAlgoLayer::executeAndProducedStatistics()
{
    // In Sequencer algorithm, the last site is not broadcasting. We build @broadcasters vector accordingly.
    std::vector<HostTuple> broadcasters = getSession()->getParam().getSites();
    broadcasters.pop_back();
    setBroadcasters(broadcasters);

    auto commLayer = getSession()->getCommLayer();
    if (getSession()->getRank() == getSession()->getParam().getSites().size() - 1)
    {
        // Process is sequencer
        auto sites = getSession()->getParam().getSites();
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Sequencer : Wait for connections on port " << get<PORT>(sites.back()) << "\n";
        commLayer->initHost(get<PORT>(sites.back()), getBroadcasters().size(), this);
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Sequencer : Wait for messages\n";
        commLayer->waitForMsg(getBroadcasters().size());
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Sequencer : Finished waiting for messages ==> Giving back control to SessionLayer\n";
        return false;
    }
    else
    {
        // Process is a broadcaster
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank()) << " : Connect to sequencer at " << get<HOSTNAME>(getSession()->getParam().getSites().back()) << ":" << get<PORT>(getSession()->getParam().getSites().back()) << "\n";
        sequencerPeer = getSession()->getCommLayer()->connectToHost(getSession()->getParam().getSites().back(), this);

        // Send RankInfo to sequencer
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank()) << " : Send RankInfo to sequencer\n";
        auto s {serializeStruct<StructRankInfo>(StructRankInfo{MsgId::RankInfo, getSession()->getRank()})};
        sequencerPeer->sendMsg(std::move(s));

        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank()) << " : Wait for messages\n";
        commLayer->waitForMsg(1); // maxDisconnections is 1, because we have only a connection with sequencer.
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank()) << " : Finished waiting for messages ==> Giving back control to SessionLayer\n";
        return true;
    }
}

void SequencerAlgoLayer::terminate() {
    // Send DisconnectIntent to sequencer
    auto s {serializeStruct<StructDisconnectIntent>(StructDisconnectIntent{MsgId::DisconnectIntent, getSession()->getRank()})};
    sequencerPeer->sendMsg(std::move(s));

}

std::string SequencerAlgoLayer::toString() {
    return "Sequencer";
}

void SequencerAlgoLayer::totalOrderBroadcast(std::string && msg) {
    // Send MessageToBroadcast to sequencer
    auto s {serializeStruct<StructMessageToBroadcast>(StructMessageToBroadcast{MsgId::MessageToBroadcast,
                                                                               getSession()->getRank(),
                                                                               std::move(msg)})};
    sequencerPeer->sendMsg(std::move(s));
}
