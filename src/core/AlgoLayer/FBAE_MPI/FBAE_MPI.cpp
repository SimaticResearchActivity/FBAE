#include "FBAE_MPI.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

#include "../../SessionLayer/SessionLayer.h"
#include "../../msgTemplates.h"

#include <mpi.h>


using namespace std;

namespace fbae::core {

FBAE_MPI::FBAE_MPI()
    : AlgoLayer(make_unique<CommLayer>(this), "fbae.core.AlgoLayer.FBAE_MPI"),
      CommLayer("fbae.core.CommLayer.FBAE_MPI") {}

#pragma region AlgoLayer

void FBAE_MPI::callbackReceiveBuffer(std::vector<char> buffer,
                                       std::vector<int> message_sizes) {
  if (algoTerminated) {
    return;
  }

  int offset = 0;

  for (int i = 0; i < sitesCount; i++) {
    string msg(buffer.data() + offset,
               buffer.data() + offset + message_sizes[i]);

    callbackReceive(move(msg));
    offset += message_sizes[i];
  }

  // Send batch
  fbae::core::AlgoLayer::BatchSessionMsg batchToSend;

  if (auto batch{batchGetBatchMsgs(rank)}; batch.has_value()) {
    batchToSend.batchSessionMsg = batch->batchSessionMsg;
    LOG4CXX_INFO_FMT(getAlgoLogger(), "{} batch send", batchToSend.batchSessionMsg.size());
  }
  else {
    LOG4CXX_INFO(getAlgoLogger(), "No batch send");
  }

  if (algoTerminated) {
    const auto serialized =
        serializeStruct<fbae::core::AlgoLayer::BatchSessionMsg>(batchToSend);
    multicastMsg(serialized);
  }
}

void FBAE_MPI::callbackReceive(std::string&& batchSessionMsgAsString) {
  auto batchSessionMsg{
      deserializeStruct<fbae::core::AlgoLayer::BatchSessionMsg>(
          std::move(batchSessionMsgAsString))};

  LOG4CXX_INFO_FMT(getAlgoLogger(), "Sender rank #{:d}",
                   batchSessionMsg.senderPos);

  for (auto sessionMessage : batchSessionMsg.batchSessionMsg) {
    batchNoDeadlockCallbackDeliver(batchSessionMsg.senderPos, sessionMessage);
  }
}

void FBAE_MPI::execute() {
  sitesCount = static_cast<int>(
      AlgoLayer::getSessionLayer()->getArguments().getSites().size());
  rank = AlgoLayer::getSessionLayer()->getRank();

  std::vector<rank_t> broadcasters(sitesCount);
  std::iota(broadcasters.begin(), broadcasters.end(), 0);
  setBroadcastersGroup(std::move(broadcasters));

  getCommLayer()->openDestAndWaitIncomingMsg(broadcasters, 0, this);
}
#pragma endregion AlgoLayer





#pragma region CommLayer
void FBAE_MPI::multicastMsg(const std::string& algoMsgAsString) {
  if (algoTerminated) {
    return;
  }

  auto msgSize = static_cast<int>(algoMsgAsString.size());

  // Gather the size of all messages from all sites
  std::vector<int> message_sizes(sitesCount);
  MPI_Allgather(&msgSize, 1, MPI_INT, message_sizes.data(), 1, MPI_INT,
                MPI_COMM_WORLD);

  // Calculate total size of all messages
  int total_message_size = 0;
  for (int i = 0; i < sitesCount; ++i) {
    total_message_size += message_sizes[i];
  }

  // Allocate buffer for receiving messages
  std::vector<char> buffer(total_message_size);

  // Calculate the offset of each messages based on their size
  std::vector<int> offsets(sitesCount);
  offsets[0] = 0;

  for (int i = 1; i < sitesCount; ++i) {
    offsets[i] = offsets[i - 1] + message_sizes[i - 1];
  }

  // Gather the messages
  MPI_Allgatherv(algoMsgAsString.data(), msgSize, MPI_BYTE, buffer.data(),
                 message_sizes.data(), offsets.data(), MPI_BYTE,
                 MPI_COMM_WORLD);

  callbackReceiveBuffer(buffer, message_sizes);
}

void FBAE_MPI::openDestAndWaitIncomingMsg(
    std::vector<rank_t> const& dest, size_t nbAwaitedConnections,
    fbae::core::AlgoLayer::AlgoLayer* aAlgoLayer) {
  setAlgoLayer(aAlgoLayer);

  // Create Comm Layer of MPI
  auto intSitesCount = static_cast<int>(sitesCount);
  auto intRank = static_cast<int>(rank);

  MPI_Comm_size(MPI_COMM_WORLD, &intSitesCount);
  MPI_Comm_rank(MPI_COMM_WORLD, &intRank);

  MPI_Barrier(MPI_COMM_WORLD);

  LOG4CXX_INFO_FMT(AlgoLayer::getAlgoLogger(),
                   "Broadcaster #{:d}; Wait for messages", rank);

  getAlgoLayer()->callbackInitDone();
}

void FBAE_MPI::send(rank_t r, const std::string& algoMsgAsString) {
  multicastMsg(algoMsgAsString);
};
#pragma endregion CommLayer

void FBAE_MPI::terminate() { algoTerminated = true; }

std::string FBAE_MPI::toString() { return "FBAE_MPI"; }
}  // namespace fbae::core