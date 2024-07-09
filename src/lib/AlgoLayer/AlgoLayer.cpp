#include "AlgoLayer.h"

#include <algorithm>

#include "../SessionLayer/SessionLayer.h"

using namespace std;
using namespace fbae_AlgoLayer;

AlgoLayer::AlgoLayer(std::unique_ptr<CommLayer> commLayer,
                     std::string const &logger_name)
    : commLayer{std::move(commLayer)}, m_logger{fbae::getLogger(logger_name)} {
  this->commLayer->setAlgoLayer(this);
}

void AlgoLayer::callbackInitDone() { sessionLayer->callbackInitDone(); }

const std::vector<rank_t> &AlgoLayer::getBroadcastersGroup() const {
  return broadcastersGroup;
}

CommLayer *AlgoLayer::getCommLayer() const { return commLayer.get(); }

std::optional<rank_t> AlgoLayer::getPosInBroadcastersGroup() const {
  auto rank = sessionLayer->getRank();
  auto lower = std::ranges::lower_bound(broadcastersGroup, rank);
  return (lower != broadcastersGroup.end() && *lower == rank)
             ? std::make_optional<rank_t>(
                   static_cast<rank_t>(lower - broadcastersGroup.begin()))
             : std::nullopt;
}

SessionLayer *AlgoLayer::getSessionLayer() const { return sessionLayer; }

bool AlgoLayer::isBroadcastingMessages() const {
  return getPosInBroadcastersGroup().has_value();
}

void AlgoLayer::setBroadcastersGroup(std::vector<rank_t> &&aBroadcastersGroup) {
  broadcastersGroup = std::move(aBroadcastersGroup);
  // As AlgoLayer::getPosInBroadcasters() and @AlgoLayer::isBroadcastingMessage
  // requires @broadcasters to be sorted, we sort it.
  std::ranges::sort(broadcastersGroup);
}

void AlgoLayer::setSessionLayer(SessionLayer *aSessionLayer) {
  sessionLayer = aSessionLayer;
}

void AlgoLayer::batchNoDeadlockCallbackDeliver(
    rank_t senderPos,
    std::shared_ptr<fbae_SessionLayer::SessionBaseClass> const &msg) {
  // We surround the call to @callbackDeliver method with shortcutBatchCtrl =
  // true; and shortcutBatchCtrl = false; This is because callbackDeliver() may
  // lead to a call to
  // @totalOrderBroadcast method which could get stuck in
  // condVarBatchCtrl.wait() instruction because task
  // @SessionLayer::sendPeriodicPerfMessage may have filled up
  // @msgsWaitingToBeBroadcast
  batchCtrlShortcut = true;
  sessionLayer->callbackDeliver(senderPos, msg);
  batchCtrlShortcut = false;
}

std::optional<fbae_AlgoLayer::BatchSessionMsg>
AlgoLayer::batchGetBatchMsgsWithLock(rank_t senderPos) {
  lock_guard lck(batchCtrlMtx);
  if (batchWaitingSessionMsg.empty()) {
    return std::nullopt;
  } else {
    auto msg = std::make_optional<fbae_AlgoLayer::BatchSessionMsg>(
        {senderPos, std::move(batchWaitingSessionMsg)});
    batchWaitingSessionMsg.clear();
    return msg;
  }
}

std::optional<fbae_AlgoLayer::BatchSessionMsg> AlgoLayer::batchGetBatchMsgs(
    rank_t senderPos) {
  auto msg{batchGetBatchMsgsWithLock(senderPos)};
  batchCtrlCondVar.notify_one();
  return msg;
}

void AlgoLayer::totalOrderBroadcast(
    const fbae_SessionLayer::SessionMsg &sessionMsg) {
  unique_lock lck(batchCtrlMtx);
  batchCtrlCondVar.wait(lck, [this] {
    return (batchWaitingSessionMsg.size() <
            getSessionLayer()->getArguments().getMaxBatchSize()) ||
           (batchCtrlShortcut &&
            std::ranges::find(batchCtrlThreadsRegisteredForFullBatchCtrl,
                              std::this_thread::get_id()) !=
                batchCtrlThreadsRegisteredForFullBatchCtrl.end());
  });
  batchWaitingSessionMsg.push_back(sessionMsg);
}

void AlgoLayer::batchRegisterThreadForFullBatchCtrl() {
  batchCtrlThreadsRegisteredForFullBatchCtrl.push_back(
      std::this_thread::get_id());
}

fbae::LoggerPtr AlgoLayer::getAlgoLogger() const { return m_logger; }

vector<fbae_SessionLayer::SessionMsg> AlgoLayer::getBatchWaitingSessionMsg()
    const {
  return batchWaitingSessionMsg;
}