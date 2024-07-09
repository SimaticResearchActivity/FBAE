#pragma once

#include "../Arguments.h"
#include "../CommLayer/CommLayer.h"
#include "../Logger/LoggerConfig.h"
#include "../basicTypes.h"
#include "SessionLayerMsg.h"

namespace fbae::core::AlgoLayer {
class AlgoLayer;
}

namespace fbae::core::SessionLayer {

class SessionLayer {
 public:
  virtual ~SessionLayer() = default;

  explicit SessionLayer(const Arguments &arguments, rank_t rank,
                        std::unique_ptr<fbae::core::AlgoLayer::AlgoLayer> algoLayer,
                        std::string const &logger_name);

  /**
   * @brief Constructor used only for tests.
   * @param rank
   */
  explicit SessionLayer(const Arguments &arguments, rank_t rank);

  /**
   * @brief Callback called by @AlgoLayer when @AlgoLayer is able to deliver
   * totalOrderBroadcast @msg.
   * @param senderPos Position of @msg sender in @AlgoLayer::broadcasters.
   * @param seqNum Sequence number of @msg.
   * @param msg Message to be delivered.
   */
  virtual void callbackDeliver(rank_t senderPos,
                               SessionMsg msg) = 0;

  /**
   * @brief Callback called by @AlgoLayer when @AlgoLayer is initialized
   * locally.
   */
  virtual void callbackInitDone() = 0;

  /**
   * @brief Entry point of @SessionLayer to execute it.
   */
  virtual void execute() = 0;

  /**
   * @brief Getter for @algolayer.
   * @return @algolayer
   */
  [[nodiscard]] fbae::core::AlgoLayer::AlgoLayer *getAlgoLayer() const;

  /**
   * @brief Getter for @arguments.
   * @return @arguments
   */
  [[nodiscard]] const Arguments &getArguments() const;

  /**
   * @brief Getter for @rank.
   * @return @rank.
   */
  [[nodiscard]] virtual rank_t getRank() const;

 protected:
  /**
   * @brief Return the logger of the parent
   */
  [[nodiscard]] fbae::core::Logger::LoggerPtr getSessionLogger() const;

 private:
  const Arguments &arguments;
  std::unique_ptr<fbae::core::AlgoLayer::AlgoLayer> algoLayer;
  const rank_t rank;

  /**
   * @brief Logger used to print informations
   */
  fbae::core::Logger::LoggerPtr m_logger;
};

}  // namespace fbae::core::SessionLayer
