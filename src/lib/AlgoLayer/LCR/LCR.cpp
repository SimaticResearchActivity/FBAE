#include <iostream>
#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "LCR.h"
#include "LCRMessage.h"
#include "../../msgTemplates.h"

using namespace fbae_LCRAlgoLayer;


LCR::LCR(std::unique_ptr<CommLayer> commLayer)
        :  vectorClock(), pending(), AlgoLayer(std::move(commLayer)) {
    // We cannot initialize the vector clock at this point in time, as we need
    // access to the session layer which is not yet initialized.
}

void LCR::initializeVectorClock() noexcept {
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();
    // Preallocate the capacity of the vector.
    vectorClock.reserve(sitesCount);

    // The vector clock is initially filled with 0s.
    for (lcr_clock_t i = 0; i < sitesCount; i++)
        vectorClock.push_back(0);
}

void LCR::tryDeliver() noexcept {
    std::cout << std::endl;
    while (pending[0].isStable) {
        getSessionLayer()->callbackDeliver(pending[0].senderRank, pending[0].sessionMessage);
        pending.erase(pending.begin());
    }
}

std::optional<StructBroadcastMessage> LCR::handleMessageReceive(StructBroadcastMessage message) noexcept {
    // Don't handle messages that are outdated.
    if (message.clock <= vectorClock[message.senderRank])
        return {};

    // Get the total process count.
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    // Get the rank of the current and next sites.
    const rank_t currentSiteRank = getSessionLayer()->getRank();
    const rank_t nextSiteRank = (currentSiteRank + 1) % sitesCount;

    // Increment the clock of the current process.
    vectorClock[message.senderRank] += 1;

    // If the next process is the same as the one who initiated the message,
    // then all processes have had the message...
    const bool cycleFinished = nextSiteRank == message.senderRank;

    // ... and if so, the message becomes stable.
    message.isStable = cycleFinished;

    // Append the message to the list of messages of the current process.
    pending.push_front(message);

    // If the cycle is finished, mark the message to be sent as an acknowledgement, and
    // to deliver pending messages.
    if (cycleFinished) {
        message.messageId = MessageId::Acknowledgement;
        tryDeliver();
    }

    // forward the message to the next site.
    return std::move(message);
}

std::optional<StructBroadcastMessage> LCR::handleAcknowledgmentReceive(StructBroadcastMessage message) noexcept {
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
    for (auto pendingMessage : pending) {
        if (pendingMessage.clock == message.clock && pendingMessage.senderRank == message.senderRank) {
            pendingMessage.isStable = true;
            // only a single message can have both checks be true, no
            // need to iterate anymore.
            break;
        }

    }

    // Try and deliver pending messages.
    tryDeliver();

    // The message to be forwarded to the next site.
    return std::move(message);
}

void LCR::callbackReceive(std::string &&algoMsgAsString) {
    // Deserialize the message.
    auto message = deserializeStruct<StructBroadcastMessage>(std::move(algoMsgAsString));

    std::cout << "Site: " << static_cast<uint32_t>(getSessionLayer()->getRank()) << " " << message << std::endl;

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
        getCommLayer()->multicastMsg(serialized);
    }
}

void LCR::execute() {
    // This initialization is done now because at this point in time
    // we have access to the session layer.
    initializeVectorClock();

    const rank_t rank = getSessionLayer()->getRank();
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    // Get the list of all broadcasters (which corresponds to all processes).
    std::vector<rank_t> broadcasters(sitesCount);
    std::iota(broadcasters.begin(), broadcasters.end(), 0);

    // Assign these broadcasters to the broadcaster group in the AlgoLayer class.
    setBroadcastersGroup(std::move(broadcasters));

    // The only destination this site has is its successor, and it only
    // expects a single other site to access this current site, hence the
    // parameters.
    getCommLayer()->openDestAndWaitIncomingMsg({ static_cast<rank_t>((rank + 1) % sitesCount) }, 1, this);
}

void LCR::terminate() {
    getCommLayer()->terminate();
}

std::string LCR::toString() {
    return "LCR";
}

void LCR::totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMessage) {
    // Get the rank of the current site.
    const rank_t currentRank = getSessionLayer()->getRank();

    // Increase the vector clock of the current site (broadcasting is an action).
    vectorClock[currentRank] += 1;

    // Construct the message struct.
    const StructBroadcastMessage message = {
            .messageId = MessageId::Message,
            .senderRank = currentRank,
            .isStable = false,
            .clock = vectorClock[currentRank],
            .sessionMessage = sessionMessage,
    };

    // Append the message to the list of pending messages to be delivered by the
    // current site.
    pending.push_front(message);

    // Serialize the message...
    auto serialized = serializeStruct<StructBroadcastMessage>(message);
    // ... and send it to the current site's successor.
    getCommLayer()->multicastMsg(serialized);
}
