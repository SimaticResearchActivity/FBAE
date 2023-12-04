#include <iostream>
#include <thread>
#include "../../SessionLayer.h"
#include "SequencerAlgoLayer.h"
#include "SequencerAlgoLayerMsg.h"
#include "../../msgTemplates.h"

using namespace std;
using namespace fbae_SequencerAlgoLayer;

bool SequencerAlgoLayer::callbackHandleMessage(std::unique_ptr<CommPeer> peer, const std::string &msgString)
{
    MsgId msgId{static_cast<MsgId>(msgString[0]) };
    switch (msgId)
    {
        using enum MsgId;
        //
        // Cases corresponding to messages received by sequencer
        //
        case DisconnectIntent :
        {
            auto bdi{deserializeStruct<StructDisconnectIntent>(msgString)};
            auto s{serializeStruct<StructAckDisconnectIntent>(StructAckDisconnectIntent{MsgId::AckDisconnectIntent})};
            peer->sendMsg(s);
            if (getSession()->getParam().getVerbose())
                cout << "\tSequencerAlgoLayer / Sequencer : Broadcaster #" << static_cast<unsigned int>(bdi.senderRank) << " announces it will disconnect.\n";
            break;
        }
        case RankInfo :
        {
            auto bri{deserializeStruct<StructRankInfo>(msgString)};
            if (getSession()->getParam().getVerbose())
                cout << "\tSequencerAlgoLayer / Sequencer : Broadcaster #" << static_cast<unsigned int>(bri.senderRank) << " is connected to sequencer.\n";
            if (++nbConnectedBroadcasters == getBroadcasters().size())
            {
                if (getSession()->getParam().getVerbose())
                    cout << "\tSequencerAlgoLayer / Sequencer : All broadcasters are connected: Broadcast AllBroadcastersConnected\n";
                auto s{serializeStruct<StructAllBroadcastersConnected >(StructAllBroadcastersConnected{MsgId::AllBroadcastersConnected})};
                getSession()->getCommLayer()->broadcastMsg(s);
            }
            break;
        }
        case MessageToBroadcast :
        {
            auto bmtb{deserializeStruct<StructMessageToBroadcast>(msgString)};
            auto s {serializeStruct<StructBroadcastMessage>(StructBroadcastMessage{MsgId::BroadcastMessage,
                                                                                   bmtb.senderRank,
                                                                                   ++seqNum,
                                                                                   bmtb.sessionMsg})};
            getSession()->getCommLayer()->broadcastMsg(s);
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
            auto sbm {deserializeStruct<StructBroadcastMessage>(msgString)};
            if (nextDeliver != sbm.seqNum)
            {
                cerr << "ERROR\tSequencerAlgoLayer / Broadcaster #" << getSession()->getRank() << " : Received a totalOrderBroadcast message with seqNum=" << sbm.seqNum << " while nextDeliver=" << nextDeliver << "\n";
                exit(EXIT_FAILURE);
            }
            getSession()->callbackDeliver(sbm.senderRank, sbm.seqNum, sbm.sessionMsg);
            ++nextDeliver;
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
            cout << "\tSequencerAlgoLayer / Broadcaster #" << getSession()->getRank() << " : Connect to sequencer at " << get<HOSTNAME>(getSession()->getParam().getSites().back()) << ":" << get<PORT>(getSession()->getParam().getSites().back()) << "\n";
        sequencerPeer = getSession()->getCommLayer()->connectToHost(getSession()->getParam().getSites().back(), this);

        // Send RankInfo to sequencer
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster #" << getSession()->getRank() << " : Send RankInfo to sequencer\n";
        auto s {serializeStruct<StructRankInfo>(StructRankInfo{MsgId::RankInfo, static_cast<unsigned char>(getSession()->getRank())})};
        sequencerPeer->sendMsg(s );

        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster #" << getSession()->getRank() << " : Wait for messages\n";
        commLayer->waitForMsg(1); // maxDisconnections is 1, because we have only a connection with sequencer.
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster #" << getSession()->getRank() << " : Finished waiting for messages ==> Giving back control to SessionLayer\n";
        return true;
    }
}

void SequencerAlgoLayer::totalOrderBroadcast(const std::string &msg) {
    // Send MessageToBroadcast to sequencer
    auto bmtb {serializeStruct<StructMessageToBroadcast>(StructMessageToBroadcast{MsgId::MessageToBroadcast,
                                                                                  static_cast<unsigned char>(getSession()->getRank()),
                                                                                  msg})};
    sequencerPeer->sendMsg(bmtb );
}

void SequencerAlgoLayer::terminate() {
    // Send DisconnectIntent to sequencer
    auto s {serializeStruct<StructDisconnectIntent>(StructDisconnectIntent{MsgId::DisconnectIntent, static_cast<unsigned char>(getSession()->getRank())})};
    sequencerPeer->sendMsg(s );

}

std::string SequencerAlgoLayer::toString() {
    return "Sequencer";
}
