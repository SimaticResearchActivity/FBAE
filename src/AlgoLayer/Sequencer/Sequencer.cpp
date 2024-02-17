#include <iostream>
#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "Sequencer.h"
#include "SequencerMsg.h"
#include "../../msgTemplates.h"

using namespace std;
using namespace fbae_SequencerAlgoLayer;

Sequencer::Sequencer(std::unique_ptr<CommLayer> aCommLayer)
        : AlgoLayer{std::move(aCommLayer)}
{
}

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
            getCommLayer()->multicastMsg(s);
            break;
        }
        //
        // Cases corresponding to messages received by a broadcaster
        //
        case Broadcast :
        {
            auto sbm {deserializeStruct<StructBroadcastMessage>(std::move(msgString))};
            getSessionLayer()->callbackDeliver(sbm.senderPos, std::move(sbm.sessionMsg));
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
    vector<rank_t> v(getSessionLayer()->getArguments().getSites().size() - 1); // -1 because sequencer is not broadcasting.
    std::iota(v.begin(), v.end(), 1); // @broadcasters must start at 1, because sequencer always has rank 0.
    setBroadcasters(std::move(v));

    // Prepare call to @CommLayer::openDestAndWaitIncomingMsg()
    if (getSessionLayer()->getRankFromRuntimeArgument() == sequencerRank)
    {
        // Process is sequencer
        getCommLayer()->openDestAndWaitIncomingMsg(getBroadcasters(), getBroadcasters().size(), this);
        if (!isBroadcastingMessage()) {
            // As the Sequencer is not broadcasting messages, it does not call getCommLayer()->terminate()
            // in a natural manner ==> We have to call it.
            getCommLayer()->terminate();
        }
        if (getSessionLayer()->getArguments().getVerbose())
            cout << "\tSequencerAlgoLayer / Sequencer : Finished waiting for messages ==> Giving back control to SessionLayer\n";
    }
    else
    {
        // Process is a broadcaster
        vector<rank_t> dest{sequencerRank};
        getCommLayer()->openDestAndWaitIncomingMsg(dest, 1, this);
        if (getSessionLayer()->getArguments().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster with rank #" << static_cast<uint32_t>(getSessionLayer()->getRankFromRuntimeArgument()) << " : Finished waiting for messages ==> Giving back control to SessionLayer\n";
    }
}

void Sequencer::terminate() {
    getCommLayer()->terminate();
}

std::string Sequencer::toString() {
    return "Sequencer";
}

void Sequencer::totalOrderBroadcast(std::string && msg) {
    // Send BroadcastRequest to sequencer
    auto s {serializeStruct<StructBroadcastMessage>(StructBroadcastMessage{MsgId::BroadcastRequest,
                                                                           getPosInBroadcasters(),
                                                                               std::move(msg)})};
    getCommLayer()->send(sequencerRank, s);
}