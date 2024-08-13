#pragma once

#include <queue>
#include <condition_variable>

#include "../CommLayer.h"

namespace fbae::core::CommLayer::Comm_MPI {

class Comm_MPI : public CommLayer {
 public:
  explicit Comm_MPI();

  void multicastMsg(const std::string& algoMsgAsString) override;

  size_t initCommLayer(fbae::core::AlgoLayer::AlgoLayer* aAlgoLayer) override;

  void openDestAndWaitIncomingMsg(std::vector<rank_t> const& dest,
                                  size_t nbAwaitedConnections) override;

  void process_Comm_MPI();

  void receive_msg_loop();

  void addMessageToQueue(std::string& msg);

  void process_msg_loop();

  void send(rank_t r, const std::string& algoMsgAsString) override;

  void terminate() override;

  void setCommTerminated();

  [[nodiscard]] std::string toString() override;

private:
  bool commTerminated = false;
  std::condition_variable queue_cv;
  std::mutex MPI_mutex;
  std::queue<std::string> message_queue;
};

}  // namespace fbae::core::CommLayer::Comm_MPI