#include "FMPI.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>
#include <future>

#include "../../SessionLayer/SessionLayer.h"
#include "../../msgTemplates.h"
#include "../../CommLayer/CommStub.h"

#include <mpi.h>


using namespace std;

namespace fbae::core::AlgoLayer::FMPI {

FMPI::FMPI()
    : AlgoLayer(make_unique<CommLayer::CommStub>(),
                "fbae.core.AlgoLayer.FMPI") {}

void FMPI::execute() {

  // Init sitesCount, rank and broadcaster group
  sitesCount = static_cast<uint32_t>(
      AlgoLayer::getSessionLayer()->getArguments().getSites().size());

  rank = AlgoLayer::getSessionLayer()->getRank();

  std::vector<rank_t> broadcasters(sitesCount);
  std::iota(broadcasters.begin(), broadcasters.end(), 0);
  setBroadcastersGroup(std::move(broadcasters));


  // Initialize MPI
  int required = MPI_THREAD_MULTIPLE;
  int provided;
  if (MPI_Init_thread(nullptr, nullptr, required, &provided) != MPI_SUCCESS) {
    LOG4CXX_FATAL(getAlgoLogger(), "Failed to initialize MPI");
    exit(EXIT_FAILURE);
  }

  if (provided < required) {
    LOG4CXX_FATAL_FMT(getAlgoLogger(),
                      "MPI does not provide required threading level");
    exit(EXIT_FAILURE);
  }

  // MPI Information
  int MPI_sitesCount;
  int MPI_rank;

  if (MPI_Comm_size(MPI_COMM_WORLD, &MPI_sitesCount) != MPI_SUCCESS) {
    LOG4CXX_FATAL_FMT(getAlgoLogger(), "Rank #{:d}: MPI_Comm_Size failed", rank);
    exit(EXIT_FAILURE);
  }

  if (MPI_Comm_rank(MPI_COMM_WORLD, &MPI_rank) != MPI_SUCCESS) {
    LOG4CXX_FATAL_FMT(getAlgoLogger(), "Rank #{:d}: MPI_Comm_rank failed", rank);
    exit(EXIT_FAILURE);
  }

  LOG4CXX_INFO_FMT(
      getAlgoLogger(),
      "Rank: #{:d}: MPI_rank: {}; SitesCount: {}; MPI_sitesCount: {}", rank,
      MPI_rank, sitesCount, MPI_sitesCount);

  sitesCount = static_cast<uint32_t>(MPI_sitesCount);
  rank = static_cast<rank_t>(MPI_rank);
  
  // Wait until all tasks joined MPI
  MPI_Barrier(MPI_COMM_WORLD);

  LOG4CXX_INFO_FMT(getAlgoLogger(), "Rank #{:d}: All joined", rank);

  callbackInitDone();
}

void FMPI::callbackInitDone() {
  AlgoLayer::callbackInitDone();

  processFMPI();
}

void FMPI::processFMPI() {

  auto task_to_receive_msg = std::async(std::launch::async, [this] {

    while (!algoTerminated) {
      string batchToSendString = createBatchToSend();

      ReceivedBuffer receivedBuffer = sendAndReceive(batchToSendString);

      readBuffer(receivedBuffer.buffer, receivedBuffer.message_sizes);
    }
  });
  task_to_receive_msg.get();
}

string FMPI::createBatchToSend() {
  if (algoTerminated) {
    return "";
  }

  // Send batch
  BatchSessionMsg batchToSend;

  if (auto batch{batchGetBatchMsgs(rank)}; batch.has_value()) {
    batchToSend.batchSessionMsg = batch->batchSessionMsg;
    LOG4CXX_INFO_FMT(getAlgoLogger(),
                     "Rank #{:d}: Batch to send with {} message", rank,
                     batchToSend.batchSessionMsg.size());
  } else {
    LOG4CXX_INFO_FMT(getAlgoLogger(), "Rank #{:d}: No batch to send", rank);
  }

  return serializeStruct<BatchSessionMsg>(batchToSend);  
}

ReceivedBuffer FMPI::sendAndReceive(const std::string_view& algoMsgAsString) {
  if (algoTerminated) {
    return ReceivedBuffer{vector<char>(), vector<int>()};
  }

  auto msgSize = static_cast<int>(algoMsgAsString.size());

  // Gather the size of all messages from all sites
  std::vector<int> message_sizes(sitesCount);

  MPI_Allgather(&msgSize, 1, MPI_INT, message_sizes.data(), 1, MPI_INT,
                MPI_COMM_WORLD);

  LOG4CXX_INFO_FMT(getAlgoLogger(),
                   "Rank #{:d}: Messages sizes: [ {}, {}, {} ]", rank,
                   message_sizes[0], message_sizes[1], message_sizes[2]);

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

  LOG4CXX_INFO_FMT(getAlgoLogger(), "Rank #{:d}: All messages gathered", rank);

  return ReceivedBuffer{.buffer = buffer, .message_sizes = message_sizes};
}

void FMPI::readBuffer(std::vector<char> buffer,
  std::vector<int> message_sizes) {
  if (algoTerminated) {
    return;
  }

  int offset = 0;
  for (int i = 0; i < sitesCount; i++) {
    if (message_sizes[i] != 0) {
      string msg(buffer.data() + offset,
                 buffer.data() + offset + message_sizes[i]);

      callbackReceive(move(msg));
    }
    offset += message_sizes[i];
  }
}

void FMPI::callbackReceive(std::string&& batchSessionMsgAsString) {
  auto batchSessionMsg{
      deserializeStruct<BatchSessionMsg>(
          std::move(batchSessionMsgAsString))};

  LOG4CXX_INFO_FMT(getAlgoLogger(), "Sender rank #{:d}",
                   batchSessionMsg.senderPos);

  for (auto const& sessionMessage : batchSessionMsg.batchSessionMsg) {
    batchNoDeadlockCallbackDeliver(batchSessionMsg.senderPos, sessionMessage);
  }
}

void FMPI::terminate() {
  algoTerminated = true;
  MPI_Finalize();
}

std::string FMPI::toString() { return "FMPI"; }
}  // namespace fbae::core::AlgoLayer::FMPI