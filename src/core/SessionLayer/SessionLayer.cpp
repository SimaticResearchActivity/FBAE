#include "SessionLayer.h"

#include "../AlgoLayer/AlgoLayer.h"

SessionLayer::SessionLayer(const Arguments &arguments, rank_t rank,
                           std::unique_ptr<AlgoLayer> algoLayer,
                           std::string const &logger_name)
    : arguments{arguments},
      algoLayer{std::move(algoLayer)},
      rank{rank},
      m_logger{fbae::getLogger(logger_name)} {
  this->algoLayer->setSessionLayer(this);
}

SessionLayer::SessionLayer(const Arguments &arguments, rank_t rank)
    : arguments{arguments}, algoLayer{nullptr}, rank{rank} {}

AlgoLayer *SessionLayer::getAlgoLayer() const { return algoLayer.get(); }

const Arguments &SessionLayer::getArguments() const { return arguments; }

rank_t SessionLayer::getRank() const { return rank; }

fbae::LoggerPtr SessionLayer::getSessionLogger() const { return m_logger; }