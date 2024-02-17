#include <algorithm>
#include <cassert>
#include "AlgoLayer.h"
#include "../SessionLayer/SessionLayer.h"

void AlgoLayer::callbackInitDone() {
    sessionLayer->callbackInitDone();
}

const std::vector<rank_t> & AlgoLayer::getBroadcasters() const {
    return broadcasters;
}

rank_t AlgoLayer::getPosInBroadcasters() const {
    auto rank = sessionLayer->getRankFromRuntimeArgument();
    auto lower = std::ranges::lower_bound(broadcasters, rank);
    assert(lower != broadcasters.end() && *lower == rank);
    return static_cast<rank_t>(lower - broadcasters.begin());
}

SessionLayer* AlgoLayer::getSessionLayer() const {
    return sessionLayer;
}

void AlgoLayer::setBroadcasters(std::vector<rank_t> &&aBroadcasters) {
    broadcasters = std::move(aBroadcasters);
    // As AlgoLayer::getPosInBroadcasters() and @AlgoLayer::isBroadcastingMessage requires @broadcasters to be sorted, we sort it.
    std::ranges::sort(broadcasters);
}

void AlgoLayer::setSessionLayer(SessionLayer *aSessionLayer)
{
    sessionLayer = aSessionLayer;
}

bool AlgoLayer::isBroadcastingMessage() const {
    return std::ranges::binary_search(broadcasters, getSessionLayer()->getRankFromRuntimeArgument());
}