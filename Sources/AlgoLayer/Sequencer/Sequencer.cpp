#include <iostream>
#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "Sequencer.h"
#include "SequencerMsg.h"
#include "../../msgTemplates.h"

using namespace std;
using namespace fbae_SequencerAlgoLayer;

void Sequencer::callbackHandleMessage(std::string && msgString)
{
    auto msgId{ static_cast<MsgId>(msgString[0]) };
    switch (msgId)
    {
        using enum MsgId;
        //
        // Cases corresponding to messages received by sequencer
        //
        case BroadcastRequest :
        {
            auto msgToBroadcast{deserializeStruct<StructBroadcastMessage>(std::move(msgString))};
            auto s {serializeStruct<StructBroadcastMessage>(StructBroadcastMessage{MsgId::Broadcast,
                                                                                   msgToBroadcast.senderPos,
                                                                                   msgToBroadcast.sessionMsg})};
            getSession()->getCommLayer()->multicastMsg(std::move(s));
            break;
        }
        //
        // Cases corresponding to messages received by a broadcaster
        //
        case Broadcast :
        {
            auto sbm {deserializeStruct<StructBroadcastMessage>(std::move(msgString))};
            getSession()->callbackDeliver(sbm.senderPos, std::move(sbm.sessionMsg));
            break;
        }
        default:
        {
            cerr << "ERROR\tSequencerAlgoLayer: Unexpected msgId (" << static_cast<int>(msgId) << ")\n";
            exit(EXIT_FAILURE);
        }
    }
}

void Sequencer::execute()
{
    // Compute vector of broadcasters rank
    vector<rank_t> v(getSession()->getParam().getSites().size() - 1); // -1 because sequencer is not broadcasting.
    std::iota(v.begin(), v.end(), 1); // @broadcasters must start at 1, because sequencer always has rank 0.
    setBroadcasters(std::move(v));

    // Prepare call to @CommLayer::openDestAndWaitIncomingMsg()
    if (getSession()->getRankFromRuntimeArgument() == sequencerRank)
    {
        // Process is sequencer
        getSession()->getCommLayer()->openDestAndWaitIncomingMsg(getBroadcasters(), getBroadcasters().size(), this);
        if (!isBroadcastingMessage()) {
            // As the Sequencer is not broadcasting messages, it does not call getSession()->getCommLayer()->terminate()
            // in a natural manner ==> We have to call it.
            getSession()->getCommLayer()->terminate();
        }
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Sequencer : Finished waiting for messages ==> Giving back control to SessionLayer\n";
    }
    else
    {
        // Process is a broadcaster
        vector<rank_t> dest{sequencerRank};
        getSession()->getCommLayer()->openDestAndWaitIncomingMsg(dest, 1, this);
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster with rank #" << static_cast<uint32_t>(getSession()->getRankFromRuntimeArgument()) << " : Finished waiting for messages ==> Giving back control to SessionLayer\n";
    }
}

void Sequencer::terminate() {
    getSession()->getCommLayer()->terminate();
}

std::string Sequencer::toString() {
    return "Sequencer";
}

void Sequencer::totalOrderBroadcast(std::string && msg) {
    // Send BroadcastRequest to sequencer
    auto s {serializeStruct<StructBroadcastMessage>(StructBroadcastMessage{MsgId::BroadcastRequest,
                                                                           getPosInBroadcasters(),
                                                                               std::move(msg)})};
    getSession()->getCommLayer()->send(sequencerRank, std::move(s));
}
