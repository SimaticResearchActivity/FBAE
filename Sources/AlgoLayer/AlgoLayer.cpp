#include <algorithm>
#include <cassert>
#include "AlgoLayer.h"
#include "../SessionLayer/SessionLayer.h"

void AlgoLayer::callbackInitDone() {
    session->callbackInitDone();
}

const std::vector<rank_t> & AlgoLayer::getBroadcasters() const {
    return broadcasters;
}

rank_t AlgoLayer::getPosInBroadcasters() const {
    auto rank = session->getRankFromRuntimeArgument();
    auto lower = std::ranges::lower_bound(broadcasters, rank);
    assert(lower != broadcasters.end() && *lower == rank);
    return static_cast<rank_t>(lower - broadcasters.begin());
}

SessionLayer* AlgoLayer::getSession() const {
    return session;
}

void AlgoLayer::setBroadcasters(std::vector<rank_t> &&aBroadcasters) {
    broadcasters = aBroadcasters;
    // As @AlgoLayer::isBroadcastingMessage requires @broadcasters to be sorted, we sort it.
    std::ranges::sort(broadcasters);
    assert(broadcasters[0] == 0); // @broadcasters must always start with 0, if we want @Session::processPerfMeasureMsg() to work properly.
}

void AlgoLayer::setSession(SessionLayer *aSession)
{
    session = aSession;
}

bool AlgoLayer::isBroadcastingMessage() const {
    return std::ranges::binary_search(broadcasters, getSession()->getRankFromRuntimeArgument());
}