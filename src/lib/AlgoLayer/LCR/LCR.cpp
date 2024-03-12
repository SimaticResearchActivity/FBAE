#include <iostream>
#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "LCR.h"
#include "LCRMessage.h"
#include "../../msgTemplates.h"

using namespace fbae_LCRAlgoLayer;

LCRLayer::LCRLayer(std::unique_ptr<CommLayer> commLayer)
        :  vectorClock(), pending(), AlgoLayer(std::move(commLayer)) {

    // Initialize the vector clock.

    // Get the sites count.
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    // Preallocate the capacity of the vector.
    vectorClock.reserve(sitesCount);

    // The vector clock is initially filled with 0s.
    for (uint32_t i = 0; i < sitesCount; i++)
        vectorClock.push_back(0);
}

void LCRLayer::tryDeliver() {
    while (pending[0].isStable) {
        // TODO, I don't know what to do here.
        std::cout << "We delivered a message!\n";
        pending.erase(pending.begin());
    }
}

std::optional<StructBroadcastMessage> LCRLayer::handleMessageReceive(StructBroadcastMessage message) {
    // Get the total process count.
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    // Get the rank of the current and next sites.
    const rank_t currentSiteRank = getSessionLayer()->getRank();
    const rank_t nextSiteRank = (currentSiteRank + 1) % sitesCount;

    // Get the sender's time on the message and the current process's clocks.
    const uint32_t messageClock = message.vectorClock[message.senderRank];
    const uint32_t currentClock = vectorClock[message.senderRank];

    // TODO: I did not understand what this was supposed to do.
    // If the message clock is earlier than the current clock, do nothing and
    // do not forward any message.
    if (messageClock <= currentClock)
        return {};

    // Increment the clock of the current process.
    vectorClock[message.senderRank] += 1;

    // If the next process is the same as the one who initiated the message,
    // then all processes have had the message...
    const bool cycleFinished = nextSiteRank == message.senderRank;
    // ... and if so, the message becomes stable.
    message.isStable = cycleFinished;

    // Append the message to the list of messages of the current process.
    pending.push_back(message);

    // If the cycle is finished, the message should become an acknowledgment message.
    // if not, it should stay as-is.
    message.messageId = cycleFinished ? MessageId::Acknowledgement : MessageId::Message;

    // Try and deliver pending messages if the cycle is complete.
    if (cycleFinished)
        tryDeliver();

    // The message to be forwarded to the next site.
    return std::move(message);
}

std::optional<StructBroadcastMessage> LCRLayer::handleAcknowledgmentReceive(StructBroadcastMessage message) {
    // Get the total process count.
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    // Get the rank of the current site and its successor's successor.
    const rank_t currentSiteRank = getSessionLayer()->getRank();
    const rank_t nextNextSiteRank = (currentSiteRank + 2) % sitesCount;

    // If the successor's successor's rank is that of the sender, abort and do not
    // forward any message.
    if (nextNextSiteRank == message.senderRank)
        return {};

    // Iterate through all pending messages of the current process and
    // if the vector clocks are aligned, mark them as stable.
    for (auto pendingMessage : pending)
        // TODO: Understand this part.
        if (pendingMessage.vectorClock == message.vectorClock)
            pendingMessage.isStable = true;

    // Try and deliver pending messages.
    tryDeliver();

    // The message to be forwarded to the next site.
    return std::move(message);
}

void LCRLayer::callbackReceive(std::string &&algoMsgAsString) {

    // Deserialize the message.
    auto message = deserializeStruct<StructBroadcastMessage>(std::move(algoMsgAsString));

    std::optional<StructBroadcastMessage> messageToForward {};
    // Differentiate between if the message is being delivered or being acknowledged.

    switch (message.messageId) {
        case MessageId::Message:
            messageToForward = handleMessageReceive(std::move(message));
            break;
        case MessageId::Acknowledgement:
            messageToForward = handleAcknowledgmentReceive(std::move(message));
            break;
        default: {
            std::cerr << "ERROR\tLCRAlgoLayer: Unexpected messageId (" << static_cast<int>(message.messageId) << ")\n";
            exit(EXIT_FAILURE);
        }
    }

    // Serialize the process and send int to the next process.
    if (messageToForward.has_value()) {
        const auto serialized = serializeStruct<StructBroadcastMessage>(messageToForward.value());
        getCommLayer().multicastMsg(serialized);
    }
}

void LCRLayer::execute() {
    const rank_t rank = getSessionLayer()->getRank();
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    std::vector<rank_t> broadcastDestination { (rank + 1) % sitesCount };

    setBroadcastersGroup(std::move(broadcastDestination));

    getCommLayer()->openDestAndWaitIncomingMsg(getBroadcastersGroup(), 1, this);
}

void LCRLayer::terminate() {
    getCommLayer()->terminate();
}

std::string LCRLayer::toString() {
    return "LCR";
}

void LCRLayer::totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMessage) {
    // Get the number of processes.
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    // Get the rank of the sender and the receiver (which is the sender's successor).
    const rank_t currentRank = getSessionLayer()->getRank();
    const rank_t receiverRank = (currentRank + 1) % sitesCount;

    // Increase the vector clock of the sender process (broadcasting is an action).
    vectorClock[currentRank] += 1;

    // Construct the message struct.
    const StructBroadcastMessage message = {
            .messageId = MessageId::Message,
            .senderRank = currentRank,
            .isStable = false,
            .vectorClock = vectorClock,
            .sessionMessage = sessionMessage,
    };

    // Append the message to the list of pending messages to be delivered by the
    // sender process.
    pending.push_back(message);

    // Serialize the message...
    auto serialized = serializeStruct<StructBroadcastMessage>(message);
    // ... and send it to its successor.
    getCommLayer()->multicastMsg(serialized);
}
