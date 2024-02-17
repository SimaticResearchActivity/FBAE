#include "SessionLayer.h"

SessionLayer::SessionLayer(const Arguments &aArguments)
    : arguments{aArguments}
{
}

const Arguments &SessionLayer::getArguments() const {
    return arguments;
}