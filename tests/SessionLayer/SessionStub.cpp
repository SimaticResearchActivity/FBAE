//
// Created by simatic on 2/17/24.
//

#include <cassert>
#include "SessionStub.h"

SessionStub::SessionStub(const Arguments &arguments, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer)
        : SessionLayer(arguments, rank, std::move(algoLayer))
{
}

void SessionStub::callbackDeliver(rank_t senderPos, std::string &&msg) {
    delivered.emplace_back(senderPos, std::move(msg));
}

void SessionStub::callbackInitDone() {
    callbackInitDoneCalled = true;
}

void SessionStub::execute() {
    // No sense to call this method in the context of tests.
    assert(false);
}

std::vector<std::pair<rank_t, std::string>> &SessionStub::getDelivered() {
    return delivered;
}

bool SessionStub::isCallbackInitdoneCalled() const {
    return callbackInitDoneCalled;
}