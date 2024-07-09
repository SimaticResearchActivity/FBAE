#pragma once

#include "../AlgoLayer.h"

namespace fbae::core::AlgoLayer::Sequencer {

/**
 * @brief Rank of sequencer (which is systematically 0).
 */
constexpr static rank_t sequencerRank{0};

class Sequencer : public AlgoLayer {
 public:
  explicit Sequencer(std::unique_ptr<fbae::core::CommLayer::CommLayer> commLayer);
  void callbackReceive(std::string &&algoMsgAsString) override;
  void execute() override;
  void totalOrderBroadcast(
      const fbae::core::SessionLayer::SessionMsg &sessionMsg) override;
  void terminate() override;
  std::string toString() override;
};

}  // namespace fbae::core::AlgoLayer::Sequencer