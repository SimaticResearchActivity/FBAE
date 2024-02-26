//
// Created by simatic on 2/17/24.
//

#include <cassert>
#include "SessionStub.h"
#include "../../src/msgTemplates.h"

SessionStub::SessionStub(const Arguments &arguments, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer)
        : SessionLayer(arguments, rank, std::move(algoLayer))
{
}

void SessionStub::callbackDeliver(rank_t senderPos, fbaeSL::SessionMsg msg) {
    delivered.emplace_back(senderPos, msg);
}

void SessionStub::callbackInitDone() {
    callbackInitDoneCalled = true;
    // We simulate the sending fo FirstBroadcast as done in @PerfMeasures class.
    if (getAlgoLayer()->isBroadcastingMessages()) {
        auto sessionMsg = std::make_shared<fbaeSL::SessionFirstBroadcast>(fbaeSL::SessionMsgId::FirstBroadcast);
        getAlgoLayer()->totalOrderBroadcast(sessionMsg);
    }
}

void SessionStub::execute() {
    // No sense to call this method in the context of tests.
    assert(false);
}

std::vector<std::pair<rank_t, fbaeSL::SessionMsg>> & SessionStub::getDelivered() {
    return delivered;
}

bool SessionStub::isCallbackInitDoneCalled() const {
    return callbackInitDoneCalled;
}