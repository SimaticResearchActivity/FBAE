#include <algorithm>
#include <cassert>
#include "AlgoLayer.h"
#include "../SessionLayer/SessionLayer.h"

void AlgoLayer::callbackInitDone() {
    session->callbackInitDone();
}

const std::vector<rank_t> & AlgoLayer::getBroadcastersRank() const {
    return broadcastersRank;
}

SessionLayer* AlgoLayer::getSession() const {
    return session;
}

void AlgoLayer::setBroadcastersRank(std::vector<rank_t> &&aBroadcasters) {
    broadcastersRank = aBroadcasters;
    // As @AlgoLayer::isBroadcastingMessage requires @broadcastersRank to be sorted, we sort it.
    std::ranges::sort(broadcastersRank);
    assert(broadcastersRank[0] == 0); // @broadcastersRank must always start with 0, if we want @Session::processPerfMeasureMsg() to work properly.
}

void AlgoLayer::setSession(SessionLayer *aSession)
{
    session = aSession;
}

bool AlgoLayer::isBroadcastingMessage() const {
    return std::ranges::binary_search(broadcastersRank, getSession()->getRankFromRuntimeArgument());
}
