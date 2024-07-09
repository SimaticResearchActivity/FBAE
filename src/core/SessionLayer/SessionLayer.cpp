#include "SessionLayer.h"

#include "../AlgoLayer/AlgoLayer.h"

using namespace fbae::core;

namespace fbae::core::SessionLayer {

SessionLayer::SessionLayer(const Arguments &arguments, rank_t rank,
                           std::unique_ptr<AlgoLayer::AlgoLayer> algoLayer,
                           std::string const &logger_name)
    : arguments{arguments},
      algoLayer{std::move(algoLayer)},
      rank{rank},
      m_logger{Logger::getLogger(logger_name)} {
  this->algoLayer->setSessionLayer(this);
}

SessionLayer::SessionLayer(const Arguments &arguments, rank_t rank)
    : arguments{arguments}, algoLayer{nullptr}, rank{rank} {}

AlgoLayer::AlgoLayer *SessionLayer::getAlgoLayer() const { return algoLayer.get(); }

const Arguments &SessionLayer::getArguments() const { return arguments; }

rank_t SessionLayer::getRank() const { return rank; }

Logger::LoggerPtr SessionLayer::getSessionLogger() const { return m_logger; }

}  // namespace fbae::core::SessionLayer
