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

std::optional<StructBroadcastMessage> LCR::handleMessageReceive(StructBroadcastMessage message) noexcept {
    // Get the total process count.
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    // Get the rank of the current and next sites.
    const rank_t currentSiteRank = getSessionLayer()->getRank();
    const rank_t nextSiteRank = (currentSiteRank + 1) % sitesCount;

    // Increment the clock of the current process.
    vectorClock[message.senderRank] += 1;

    const bool cycleFinished = nextSiteRank == message.senderRank;

    if (cycleFinished) {
        getSessionLayer()->callbackDeliver(message.senderRank, message.sessionMessage);
        message.messageId = MessageId::Acknowledgement;
    } else {
        pending.push_back(message);
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

    // Find, remove and call the callbackDeliver method on sessionLayer.
    for (uint32_t i = 0; i < pending.size(); i++) {
        if (pending[i].clock == message.clock && pending[i].senderRank == message.senderRank) {
            getSessionLayer()->callbackDeliver(pending[i].senderRank, pending[i].sessionMessage);
            pending.erase(pending.begin() + i);
            break;
        }
    }

    // Try and deliver pending messages.
//    tryDeliver();

    // The message to be forwarded to the next site.
    return std::move(message);
}

void LCR::callbackReceive(std::string &&algoMsgAsString) {
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

    std::cout << "Site: " << static_cast<uint32_t>(getSessionLayer()->getRank()) << ": ";
    for (const auto &pendingMessage : pending) {
        std::cout << pendingMessage << ", ";
    }
    std::cout << std::endl;

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
    pending.push_back(message);

    // Serialize the message...
    auto serialized = serializeStruct<StructBroadcastMessage>(message);
    // ... and send it to the current site's successor.
    getCommLayer()->multicastMsg(serialized);
}
