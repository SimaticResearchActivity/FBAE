#pragma once

#include "../../adaptCereal.h"
#include "../AlgoLayer.h"
#include "../../CommLayer/CommLayer.h"

namespace fbae::core {

class FBAE_MPI : public AlgoLayer::AlgoLayer, public CommLayer::CommLayer {
 public:
  explicit FBAE_MPI();
    
  // AlgoLayer
  void callbackReceiveBuffer(std::vector<char> buffer,
                             std::vector<int> offset);

  void callbackReceive(std::string&& batchSessionMsgAsString) override;

  void execute() override;

  // CommLayer
  /*
    Not Really a multicasting here, because it send and receive data at the same time
  */
  void multicastMsg(const std::string& algoMsgAsString) override;

  void openDestAndWaitIncomingMsg(
      std::vector<rank_t> const& dest, size_t nbAwaitedConnections,
      fbae::core::AlgoLayer::AlgoLayer* aAlgoLayer) override;

  void send(rank_t r, const std::string& algoMsgAsString) override;

  // Common
  void terminate() override;

  [[nodiscard]] std::string toString() override;

  private:
  bool algoTerminated = false;

   /**
    * @brief Rank of machine
    */
   rank_t rank = 0;

   /**
    * @brief Number of machines
    */
   uint32_t sitesCount = 0;
};

}  // namespace fbae::core
