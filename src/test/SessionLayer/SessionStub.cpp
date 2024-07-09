//
// Created by simatic on 2/17/24.
//

#include "SessionStub.h"

#include <cassert>

#include "../core/msgTemplates.h"

SessionStub::SessionStub(const Arguments &arguments, rank_t rank,
                         std::unique_ptr<AlgoLayer> algoLayer)
    : SessionLayer(arguments, rank, std::move(algoLayer),
                   "fbae.session.phony") {}

void SessionStub::callbackDeliver(rank_t senderPos,
                                  fbae_SessionLayer::SessionMsg msg) {
  delivered.emplace_back(senderPos, msg);
}

void SessionStub::callbackInitDone() {
  callbackInitDoneCalled = true;
  // We simulate the sending fo FirstBroadcast as done in @PerfMeasures class.
  if (getAlgoLayer()->isBroadcastingMessages()) {
    auto sessionMsg =
        std::make_shared<fbae_SessionLayer::SessionFirstBroadcast>(
            fbae_SessionLayer::SessionMsgId::FirstBroadcast);
    getAlgoLayer()->totalOrderBroadcast(sessionMsg);
  }
}

void SessionStub::execute() {
  // No sense to call this method in the context of tests.
  assert(false);
}

std::vector<std::pair<rank_t, fbae_SessionLayer::SessionMsg>>
    &SessionStub::getDelivered() {
  return delivered;
}

bool SessionStub::isCallbackInitDoneCalled() const {
  return callbackInitDoneCalled;
}