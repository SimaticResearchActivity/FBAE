#pragma once

#include "../AlgoLayer.h"

namespace fbae::core::AlgoLayer::Algo_MPI {

struct ReceivedBuffer {
  std::vector<char> buffer;
  std::vector<int> message_sizes;
};

class Algo_MPI : public AlgoLayer {
 public:
  explicit Algo_MPI();
  
  void execute() override;

  void process_Algo_MPI();

  std::string createBatchToSend();

  [[nodiscard]] ReceivedBuffer sendAndReceive(std::string_view const& algoMsgAsString) const;

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

}  // namespace fbae::core::AlgoLayer::Algo_MPI
