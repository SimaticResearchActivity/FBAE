#pragma once

#include "../../adaptCereal.h"
#include "../AlgoLayer.h"
#include "../../CommLayer/CommLayer.h"

namespace fbae::core::AlgoLayer::FMPI {

class FMPI : public AlgoLayer {
 public:
  explicit FMPI();
  
  /**
   * @brief Deliver received messages and create message to send
   * @param buffer buffer of the messages received
   * @param message_sizes sizes of the messages received
   */
  void callbackReceiveBuffer(std::vector<char> buffer,
                             std::vector<int> message_sizes);

  void callbackReceive(std::string&& batchSessionMsgAsString) override;

  void execute() override;

  /**
   * @brief Send and receive informations to/from other sites
   * @param algoMsgAsString message to send
   */
  void sendAndReceive(const std::string_view& algoMsgAsString);

  void callbackInitDone() override;

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

}  // namespace fbae::core::AlgoLayer::FMPI
