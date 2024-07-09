#include "CommLayer.h"

using namespace fbae::core;

namespace fbae::core::CommLayer {

using fbae::core::Logger::LoggerPtr;

CommLayer::CommLayer(std::string const &logger_name)
    : m_logger{Logger::getLogger(logger_name)} {}

AlgoLayer::AlgoLayer *CommLayer::getAlgoLayer() const { return algoLayer; }

std::latch &CommLayer::getInitDoneCalled() { return initDoneCalled; }

void CommLayer::setAlgoLayer(AlgoLayer::AlgoLayer *aAlgoLayer) { algoLayer = aAlgoLayer; }

LoggerPtr CommLayer::getCommLogger() const { return m_logger; }

}  // namespace fbae::core::CommLayer
