#include <algorithm>
#include "AlgoLayer.h"
#include "../SessionLayer/SessionLayer.h"

AlgoLayer::AlgoLayer(std::unique_ptr<CommLayer> commLayer)
    : commLayer{std::move(commLayer)}
{
    this->commLayer->setAlgoLayer(this);
}

void AlgoLayer::callbackInitDone() {
    sessionLayer->callbackInitDone();
}

const std::vector<rank_t> & AlgoLayer::getBroadcastersGroup() const {
    return broadcastersGroup;
}

CommLayer *AlgoLayer::getCommLayer() const
{
    return commLayer.get();
}

std::optional<rank_t> AlgoLayer::getPosInBroadcastersGroup() const {
    auto rank = sessionLayer->getRank();
    auto lower = std::ranges::lower_bound(broadcastersGroup, rank);
    return  (lower != broadcastersGroup.end() && *lower == rank) ?
            std::make_optional<rank_t>(static_cast<rank_t>(lower - broadcastersGroup.begin())) :
            std::nullopt;
}

SessionLayer* AlgoLayer::getSessionLayer() const {
    return sessionLayer;
}

bool AlgoLayer::isBroadcastingMessages() const {
    return getPosInBroadcastersGroup().has_value();
}

void AlgoLayer::setBroadcastersGroup(std::vector<rank_t> &&aBroadcastersGroup) {
    broadcastersGroup = std::move(aBroadcastersGroup);
    // As AlgoLayer::getPosInBroadcasters() and @AlgoLayer::isBroadcastingMessage requires @broadcasters to be sorted, we sort it.
    std::ranges::sort(broadcastersGroup);
}

void AlgoLayer::setSessionLayer(SessionLayer *aSessionLayer)
{
    sessionLayer = aSessionLayer;
}