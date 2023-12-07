#include "CommLayer.h"

AlgoLayer *CommLayer::getAlgoLayer() const {
    return algoLayer;
}

void CommLayer::setAlgoLayer(AlgoLayer *aAlgoLayer) {
    algoLayer = aAlgoLayer;
}
