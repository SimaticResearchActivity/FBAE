#include "LCR.h"

#include <numeric>

#include "../../SessionLayer/SessionLayer.h"
#include "../../msgTemplates.h"

using namespace fbae::core;
using namespace fbae::core::AlgoLayer::LCR;

namespace fbae::core::AlgoLayer::LCR {

LCR::LCR(std::unique_ptr<CommLayer::CommLayer> commLayer) noexcept
    : AlgoLayer{std::move(commLayer), "fbae.core.AlgoLayer.LCR"} {
  // We cannot initialize the vector clock at this point in time, as we need
  // access to the session layer which is not yet initialized.
}

inline void LCR::initializeVectorClock() noexcept {
  vectorClock.reserve(sitesCount);
  for (LCRClock_t i = 0; i < sitesCount; i++) vectorClock.push_back(0);
}

void LCR::tryDeliver() noexcept {
  while (!pending.empty() && pending[0].isStable) {
    getSessionLayer()->callbackDeliver(pending[0].senderRank,
                                       pending[0].sessionMessage);
    pending.erase(pending.begin());
  }
}

inline std::optional<MessagePacket> LCR::handleMessageReceive(
    MessagePacket message) noexcept {
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

inline std::optional<MessagePacket> LCR::handleAcknowledgmentReceive(
    MessagePacket message) noexcept {
  if (nextSiteRank == message.senderRank) return {};

  for (auto &pendingMessage : pending) {
    if (pendingMessage.clock == message.clock &&
        pendingMessage.senderRank == message.senderRank) {
      pendingMessage.isStable = true;
      tryDeliver();
      break;
    }
  }

  return std::move(message);
}

void LCR::callbackReceive(std::string &&serializedMessagePacket) noexcept {
  auto message =
      deserializeStruct<MessagePacket>(std::move(serializedMessagePacket));

  std::optional<MessagePacket> messageToForward = {};
  switch (message.messageId) {
    case MessageId::Message:
      messageToForward = handleMessageReceive(std::move(message));
      break;
    case MessageId::Acknowledgement:
      messageToForward = handleAcknowledgmentReceive(std::move(message));
      break;
    default: {
      LOG4CXX_FATAL_FMT(getAlgoLogger(), "Unexpected messageId #{}",
                        static_cast<uint32_t>(message.messageId));
      exit(EXIT_FAILURE);
    }
  }

  if (messageToForward.has_value()) {
    const auto serialized =
        serializeStruct<MessagePacket>(messageToForward.value());
    getCommLayer()->multicastMsg(serialized);
  }
}

void LCR::execute() noexcept {
  sitesCount = static_cast<uint32_t>(getCommLayer()->initCommLayer(this));
  currentSiteRank = getSessionLayer()->getRank();
  nextSiteRank = static_cast<rank_t>((currentSiteRank + 1) % sitesCount);


  // This initialization is done now because at this point in time
  // we have access to the session layer.
  initializeVectorClock();

  std::vector<rank_t> broadcasters(sitesCount);
  std::iota(broadcasters.begin(), broadcasters.end(), 0);
  setBroadcastersGroup(std::move(broadcasters));

  getCommLayer()->openDestAndWaitIncomingMsg(
      {nextSiteRank}, 1);
}

void LCR::terminate() noexcept { getCommLayer()->terminate(); }

std::string LCR::toString() noexcept { return "LCR"; }

void LCR::totalOrderBroadcast(
    const fbae::core::SessionLayer::SessionMsg &sessionMessage) noexcept {
  vectorClock[currentSiteRank] += 1;

  const MessagePacket message = {
      .messageId = MessageId::Message,
      .senderRank = currentSiteRank,
      .clock = vectorClock[currentSiteRank],
      .sessionMessage = sessionMessage,
      .isStable = false,
  };

  pending.push_back(message);

  const auto serialized = serializeStruct<MessagePacket>(message);
  getCommLayer()->multicastMsg(serialized);
}

}  // namespace fbae::core::AlgoLayer::LCR
