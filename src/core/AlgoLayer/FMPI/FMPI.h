#pragma once

#include "../../adaptCereal.h"
#include "../AlgoLayer.h"
#include "../../CommLayer/CommLayer.h"

namespace fbae::core::AlgoLayer::FMPI {

struct ReceivedBuffer {
  std::vector<char> buffer;
  std::vector<int> message_sizes;
};

class FMPI : public AlgoLayer {
 public:
  explicit FMPI();
  ~FMPI() final;
  
  void execute() override;

  void callbackInitDone() override;

  void processFMPI();

  std::string createBatchToSend();

  ReceivedBuffer sendAndReceive(std::string_view const& algoMsgAsString) const;

  /**
   * @brief Deliver received messages and create message to send
   * @param buffer buffer of the messages received
   * @param message_sizes sizes of the messages received
   */
  void readBuffer(std::vector<char> buffer, std::vector<int> const& message_sizes);

  void callbackReceive(std::string&& batchSessionMsgAsString) override;

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
