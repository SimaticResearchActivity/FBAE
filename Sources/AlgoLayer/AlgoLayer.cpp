#include "AlgoLayer.h"
#include "../SessionLayer/SessionLayer.h"

void AlgoLayer::callbackInitDone() {
    session->callbackInitDone();
}

const std::vector<HostTuple> &AlgoLayer::getBroadcasters() const {
    return broadcasters;
}

void AlgoLayer::setBroadcasters(const std::vector<HostTuple> &aBroadcasters) {
    broadcasters = aBroadcasters;
}

void AlgoLayer::setSession(SessionLayer *aSession)
{
    session = aSession;
}

SessionLayer* AlgoLayer::getSession() const {
    return session;
}
