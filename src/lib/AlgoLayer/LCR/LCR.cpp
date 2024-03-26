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

    vectorClock.reserve(sitesCount);
    for (lcr_clock_t i = 0; i < sitesCount; i++)
        vectorClock.push_back(0);
}

inline std::optional<StructBroadcastMessage> LCR::handleMessageReceive(StructBroadcastMessage message) noexcept {
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    const rank_t currentSiteRank = getSessionLayer()->getRank();
    const rank_t nextSiteRank = (currentSiteRank + 1) % sitesCount;

    vectorClock[message.senderRank] += 1;

    if (nextSiteRank == message.senderRank) {
        getSessionLayer()->callbackDeliver(message.senderRank, message.sessionMessage);
        message.messageId = MessageId::Acknowledgement;
    } else {
        pending.push_back(message);
    }

    return std::move(message);
}

inline std::optional<StructBroadcastMessage> LCR::handleAcknowledgmentReceive(StructBroadcastMessage message) noexcept {
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    const rank_t currentSiteRank = getSessionLayer()->getRank();
    const rank_t nextSiteRank = (currentSiteRank + 1) % sitesCount;

    if (nextSiteRank == message.senderRank)
        return {};

    for (uint32_t i = 0; i < pending.size(); i++) {
        if (pending[i].clock == message.clock && pending[i].senderRank == message.senderRank) {
            getSessionLayer()->callbackDeliver(pending[i].senderRank, pending[i].sessionMessage);
            pending.erase(pending.begin() + i);
            break;
        }
    }

    return std::move(message);
}

void LCR::callbackReceive(std::string &&algoMsgAsString) {
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

void LCR::execute() {
    // This initialization is done now because at this point in time
    // we have access to the session layer.
    initializeVectorClock();

    const rank_t rank = getSessionLayer()->getRank();
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    std::vector<rank_t> broadcasters(sitesCount);
    std::iota(broadcasters.begin(), broadcasters.end(), 0);
    setBroadcastersGroup(std::move(broadcasters));

    getCommLayer()->openDestAndWaitIncomingMsg({ static_cast<rank_t>((rank + 1) % sitesCount) }, 1, this);
}

void LCR::terminate() {
    getCommLayer()->terminate();
}

std::string LCR::toString() {
    return "LCR";
}

void LCR::totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMessage) {
    const rank_t currentRank = getSessionLayer()->getRank();
    vectorClock[currentRank] += 1;

    const StructBroadcastMessage message = {
            .messageId = MessageId::Message,
            .senderRank = currentRank,
            .clock = vectorClock[currentRank],
            .sessionMessage = sessionMessage,
    };

    pending.push_back(message);

    const auto serialized = serializeStruct<StructBroadcastMessage>(message);
    getCommLayer()->multicastMsg(serialized);
}
