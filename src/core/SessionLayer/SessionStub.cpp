//
// Created by simatic on 2/17/24.
//

#include "SessionStub.h"

#include <cassert>

#include "msgTemplates.h"

namespace fbae::core::SessionLayer {

using namespace fbae::core;
using namespace fbae::core::AlgoLayer;

using enum fbae::core::SessionLayer::SessionMsgId;

using fbae::core::AlgoLayer::AlgoLayer;

SessionStub::SessionStub(const Arguments &arguments, rank_t rank,
                         std::unique_ptr<AlgoLayer> algoLayer)
    : SessionLayer(arguments, rank, std::move(algoLayer),
                   "fbae.session.phony") {}

void SessionStub::callbackDeliver(rank_t senderPos,
                                  SessionMsg msg) {
  delivered.emplace_back(senderPos, msg);
}

void SessionStub::callbackInitDone() {
  callbackInitDoneCalled = true;
  // We simulate the sending fo FirstBroadcast as done in @PerfMeasures class.
  if (getAlgoLayer()->isBroadcastingMessages()) {
    auto sessionMsg =
        std::make_shared<SessionFirstBroadcast>(
            FirstBroadcast);
    getAlgoLayer()->totalOrderBroadcast(sessionMsg);
  }
}

void SessionStub::execute() {
  // No sense to call this method in the context of tests.
  assert(false);
}

std::vector<std::pair<rank_t, SessionMsg>>
    &SessionStub::getDelivered() {
  return delivered;
}

bool SessionStub::isCallbackInitDoneCalled() const {
  return callbackInitDoneCalled;
}

}  // namespace fbae::core::SessionLayer

