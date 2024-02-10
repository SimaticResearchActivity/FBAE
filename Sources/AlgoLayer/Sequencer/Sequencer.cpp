#include <iostream>
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
                                                                                   msgToBroadcast.senderRank,
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
            getSession()->callbackDeliver(sbm.senderRank,std::move(sbm.sessionMsg));
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
    // In Sequencer algorithm, the last site is not broadcasting. We build @broadcasters vector accordingly.
    sequencerRank = static_cast<rank_t>(getSession()->getParam().getSites().size() - 1);
    std::vector<HostTuple> broadcasters = getSession()->getParam().getSites();
    broadcasters.pop_back();
    setBroadcasters(broadcasters);

    vector<rank_t> dest;
    if (getSession()->getRank() == sequencerRank)
    {
        // Process is sequencer
        for (rank_t r = 0 ; r < sequencerRank ; ++r) {
            dest.push_back(r);
        }
        getSession()->getCommLayer()->openDestAndWaitIncomingMsg(dest, broadcasters.size(), this);
        if (!isBroadcastingMessage()) {
            // As the Sequencer is not broadcastin messages, it does not call getSession()->getCommLayer()->terminate()
            // in a natural manner ==> We have to call it.
            getSession()->getCommLayer()->terminate();
        }
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Sequencer : Finished waiting for messages ==> Giving back control to SessionLayer\n";
    }
    else
    {
        // Process is a broadcaster
        dest.push_back(sequencerRank);
        getSession()->getCommLayer()->openDestAndWaitIncomingMsg(dest, 1, this);
        if (getSession()->getParam().getVerbose())
            cout << "\tSequencerAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank()) << " : Finished waiting for messages ==> Giving back control to SessionLayer\n";
    }
}

bool Sequencer::isBroadcastingMessage() const {
    // Return true if @ALgoLayer is a broadcaster and false if it is the sequencer
    return getSession()->getRank() < getSession()->getParam().getSites().size() - 1;
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
                                                                               getSession()->getRank(),
                                                                               std::move(msg)})};
    getSession()->getCommLayer()->send(sequencerRank, std::move(s));
}
