#include "CommLayer.h"

AlgoLayer *CommLayer::getAlgoLayer() const {
    return algoLayer;
}

std::latch &CommLayer::getInitDoneCalled() {
    return initDoneCalled;
}

void CommLayer::setAlgoLayer(AlgoLayer *aAlgoLayer) {
    algoLayer = aAlgoLayer;
}
