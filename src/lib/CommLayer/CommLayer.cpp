#include "CommLayer.h"

CommLayer::CommLayer(std::string const& logger_name) :
    m_logger{ fbae::getLogger(logger_name) }
{}

AlgoLayer *CommLayer::getAlgoLayer() const {
    return algoLayer;
}

std::latch &CommLayer::getInitDoneCalled() {
    return initDoneCalled;
}

void CommLayer::setAlgoLayer(AlgoLayer *aAlgoLayer) {
    algoLayer = aAlgoLayer;
}

fbae::LoggerPtr CommLayer::getCommLogger() const {
    return m_logger;
}