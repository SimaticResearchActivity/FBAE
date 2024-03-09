#include "SessionLayer.h"
#include "../AlgoLayer/AlgoLayer.h"

SessionLayer::SessionLayer(const Arguments &arguments, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer)
    : arguments{arguments}
    , algoLayer{std::move(algoLayer)}
    , rank{rank}
{
    this->algoLayer->setSessionLayer(this);
}

SessionLayer::SessionLayer(const Arguments &arguments, rank_t rank)
        : arguments{arguments}
        , algoLayer{nullptr}
        , rank{rank}{
}

AlgoLayer * SessionLayer::getAlgoLayer() const {
    return algoLayer.get();
}

const Arguments &SessionLayer::getArguments() const {
    return arguments;
}

rank_t SessionLayer::getRank() const {
    return rank;
}
