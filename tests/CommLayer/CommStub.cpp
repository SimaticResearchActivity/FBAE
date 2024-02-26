//
// Created by simatic on 2/17/24.
//

#include <cassert>
#include "../../src/AlgoLayer/AlgoLayer.h"
#include "CommStub.h"

std::vector<rank_t> &CommStub::getConnectedDest() {
    return connectedDest;
}

size_t CommStub::getNbAwaitedConnections() const {
    return nbAwaitedConnections;
}

std::vector<std::pair<rank_t, std::string>> &CommStub::getSent() {
    return sent;
}

void CommStub::multicastMsg(const std::string &algoMsgAsString) {
    for (auto const rank: connectedDest) {
        send(rank, algoMsgAsString);
    }
}

void CommStub::openDestAndWaitIncomingMsg(const std::vector<rank_t> &dest, size_t aNbAwaitedConnections,
                                          AlgoLayer *aAlgoLayer) {
    connectedDest = dest;
    nbAwaitedConnections = aNbAwaitedConnections;
    getAlgoLayer()->callbackInitDone();
}

void CommStub::send(rank_t r, const std::string &algoMsgAsString) {
    sent.emplace_back(r, algoMsgAsString);
}

void CommStub::terminate() {
    // Empty method as @CommStub::openDestAndWaitIncomingMsg() is non-blocking when it is called by
    // @AlgoLayer::execute() ==> @AlgoLayer::execute() may call @CommStub::terminate().
}

std::string CommStub::toString() {
    // No sense to call this method in the context of tests.
    assert(false);
    return "CommStub";
}