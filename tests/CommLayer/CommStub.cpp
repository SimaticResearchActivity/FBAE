//
// Created by simatic on 2/17/24.
//

#include <cassert>
#include "CommStub.h"

std::vector<rank_t> &CommStub::getConnectedDest() {
    return connectedDest;
}

std::vector<std::pair<rank_t, std::string>> &CommStub::getSent() {
    return sent;
}

void CommStub::multicastMsg(const std::string &msg) {
    for (auto const rank: connectedDest) {
        send(rank, msg);
    }
}

void CommStub::openDestAndWaitIncomingMsg(const std::vector<rank_t> &dest, size_t nbAwaitedConnections,
                                          AlgoLayer *aAlgoLayer) {
    connectedDest = dest;
}

void CommStub::send(rank_t r, const std::string &msg) {
    sent.emplace_back(r, msg);
}

void CommStub::terminate() {
    // Empty method as @CommStub::openDestAndWaitIncomingMsg() is non-blocking when it is called by
    // @AlgoLayer::execute() ==> @AlgoLayer::execute() may call @CommStub::terminate().
}

std::string CommStub::toString() {
    // No sense to call this method in the context of tests.
    assert(false);
}

