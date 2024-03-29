#include <iostream>
#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "LCR.h"
#include "LCRMessage.h"
#include "../../msgTemplates.h"

using namespace fbae_LCRAlgoLayer;

LCR::LCR(std::unique_ptr<CommLayer> commLayer) noexcept
        :  AlgoLayer(std::move(commLayer)) {
    // We cannot initialize the vector clock at this point in time, as we need
    // access to the session layer which is not yet initialized.
}

inline void LCR::initializeVectorClock() noexcept {
    const auto sitesCount = static_cast<uint32_t>(getSessionLayer()->getArguments().getSites().size());

    vectorClock.reserve(sitesCount);
    for (LCRClock_t i = 0; i < sitesCount; i++)
        vectorClock.push_back(0);
}

void LCR::tryDeliver() noexcept {
    while (!pending.empty() && pending[0].isStable) {
        getSessionLayer()->callbackDeliver(pending[0].senderRank, pending[0].sessionMessage);
        pending.erase(pending.begin());
    }
}

inline std::optional<StructBroadcastMessage> LCR::handleMessageReceive(StructBroadcastMessage message) noexcept {
    const auto sitesCount = static_cast<uint32_t>(getSessionLayer()->getArguments().getSites().size());

    const rank_t currentSiteRank = getSessionLayer()->getRank();
    const rank_t nextSiteRank = (currentSiteRank + 1) % sitesCount;

    vectorClock[message.senderRank] += 1;

    const bool isCycleFinished = nextSiteRank == message.senderRank;
    message.isStable = isCycleFinished;
    pending.push_back(message);
    if (isCycleFinished) {
        tryDeliver();
        message.messageId = MessageId::Acknowledgement;
    }

    return std::move(message);
}

inline std::optional<StructBroadcastMessage> LCR::handleAcknowledgmentReceive(StructBroadcastMessage message) noexcept {
    const auto sitesCount = static_cast<uint32_t>(getSessionLayer()->getArguments().getSites().size());

    const rank_t currentSiteRank = getSessionLayer()->getRank();

    if (const rank_t nextSiteRank = (currentSiteRank + 1) % sitesCount;
        nextSiteRank == message.senderRank)
        return {};

    for (auto &pendingMessage : pending) {
        if (pendingMessage.clock == message.clock && pendingMessage.senderRank == message.senderRank) {
            pendingMessage.isStable = true;
            tryDeliver();
            break;
        }
    }

    return std::move(message);
}

void LCR::callbackReceive(std::string &&algoMsgAsString) noexcept {
    auto message = deserializeStruct<StructBroadcastMessage>(std::move(algoMsgAsString));

    std::optional<StructBroadcastMessage> messageToForward = {};
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

    if (messageToForward.has_value()) {
        const auto serialized = serializeStruct<StructBroadcastMessage>(messageToForward.value());
        getCommLayer()->multicastMsg(serialized);
    }
}

void LCR::execute() noexcept {
    // This initialization is done now because at this point in time
    // we have access to the session layer.
    initializeVectorClock();

    const rank_t rank = getSessionLayer()->getRank();
    const auto sitesCount = static_cast<uint32_t>(getSessionLayer()->getArguments().getSites().size());

    std::vector<rank_t> broadcasters(sitesCount);
    std::iota(broadcasters.begin(), broadcasters.end(), 0);
    setBroadcastersGroup(std::move(broadcasters));

    getCommLayer()->openDestAndWaitIncomingMsg({ static_cast<rank_t>((rank + 1) % sitesCount) }, 1, this);
}

void LCR::terminate() noexcept {
    getCommLayer()->terminate();
}

std::string LCR::toString() noexcept {
    return "LCR";
}

void LCR::totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMessage) noexcept {
    const rank_t currentRank = getSessionLayer()->getRank();
    vectorClock[currentRank] += 1;

    const StructBroadcastMessage message = {
            .messageId = MessageId::Message,
            .senderRank = currentRank,
            .clock = vectorClock[currentRank],
            .sessionMessage = sessionMessage,
            .isStable = false,
    };

    pending.push_back(message);

    const auto serialized = serializeStruct<StructBroadcastMessage>(message);
    getCommLayer()->multicastMsg(serialized);
}
