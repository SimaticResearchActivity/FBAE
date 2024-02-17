#include "SessionLayer.h"
#include "../AlgoLayer/AlgoLayer.h"

SessionLayer::SessionLayer(const Arguments &aArguments, std::unique_ptr<AlgoLayer> anAlgoLayer)
    : arguments{aArguments}
    , algoLayer{std::move(anAlgoLayer)}
{
    getAlgoLayer()->setSessionLayer(this);
}

AlgoLayer * SessionLayer::getAlgoLayer() const {
    return algoLayer.get();
}

const Arguments &SessionLayer::getArguments() const {
    return arguments;
}