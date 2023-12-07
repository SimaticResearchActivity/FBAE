#include "AlgoLayer.h"

void AlgoLayer::setSession(SessionLayer *aSession)
{
    session = aSession;
}

SessionLayer* AlgoLayer::getSession() const {
    return session;
}

const std::vector<HostTuple> &AlgoLayer::getBroadcasters() const {
    return broadcasters;
}

void AlgoLayer::setBroadcasters(const std::vector<HostTuple> &aBroadcasters) {
    broadcasters = aBroadcasters;
}
