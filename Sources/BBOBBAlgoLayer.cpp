//
// Created by lardeur on 09/10/23.
//

#include "BBOBBAlgoLayer.h"

void BBOBBAlgoLayer::callbackHandleMessageAsHost(std::unique_ptr<CommPeer> peer, const std::string &msgString) {

}

void BBOBBAlgoLayer::callbackHandleMessageAsNonHostPeer(std::unique_ptr<CommPeer> peer, const std::string &msgString) {

}

bool BBOBBAlgoLayer::executeAndProducedStatistics() {
    return false;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {

}

void BBOBBAlgoLayer::terminate() {

}

std::string BBOBBAlgoLayer::toString() {
    return std::string();
}
