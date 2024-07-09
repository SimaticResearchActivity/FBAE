//
// Created by simatic on 04/04/202424.
//

#include "AlgoStub.h"

#include <cassert>

namespace fbae::core::AlgoLayer {

using namespace fbae::core;

AlgoStub::AlgoStub(std::unique_ptr<CommLayer::CommLayer> commLayer,
                   InitDoneSupervisor &initDoneSupervisor)
    : AlgoLayer::AlgoLayer{std::move(commLayer),
                           "fbae.test.AlgoLayer.AlgoStub"},
      initDoneSupervisor{initDoneSupervisor} {}

void AlgoStub::callbackReceive(std::string &&algoMsgAsString) {
  received.emplace_back(std::move(algoMsgAsString));
}

void AlgoStub::callbackInitDone() { initDoneSupervisor.callbackInitDone(); }

void AlgoStub::execute() {
  // No sense to call this method in the context of tests.
  assert(false);
}

std::vector<std::string> &AlgoStub::getReceived() { return received; }

void AlgoStub::totalOrderBroadcast(
    const SessionLayer::SessionMsg &sessionMsg) {
  // No sense to call this method in the context of tests.
  assert(false);
}

void AlgoStub::terminate() {
  // No sense to call this method in the context of tests.
  assert(false);
}

std::string AlgoStub::toString() {
  // No sense to call this method in the context of tests.
  assert(false);
  return "AlgoStub";
}

}  // namespace fbae::core::AlgoLayer
