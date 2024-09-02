#include "Comm_MPI.h"

#include <sys/stat.h>

#include <future>

#include "../../AlgoLayer/AlgoLayer.h"
#include "../../SessionLayer/SessionLayer.h"
#include "mpi.h"

using namespace std;

namespace fbae::core::CommLayer::Comm_MPI {

Comm_MPI::Comm_MPI() : CommLayer{"fbae.core.CommLayer.Comm_MPI"} {}

size_t Comm_MPI::initCommLayer(fbae::core::AlgoLayer::AlgoLayer* aAlgoLayer) {
  setAlgoLayer(aAlgoLayer);

  // Initialize MPI
  int constexpr required = MPI_THREAD_MULTIPLE;
  int provided;
  if (MPI_Init_thread(nullptr, nullptr, required, &provided) != MPI_SUCCESS) {
    LOG4CXX_FATAL(getCommLogger(), "Failed to initialize MPI");
    exit(EXIT_FAILURE);
  }

  if (provided < required) {
    LOG4CXX_FATAL(getCommLogger(),
                      "MPI does not provide required threading level");
    exit(EXIT_FAILURE);
  }

  // MPI Information
  int MPI_sitesCount;
  int MPI_rank;

  if (MPI_Comm_size(MPI_COMM_WORLD, &MPI_sitesCount) != MPI_SUCCESS) {
    LOG4CXX_FATAL(getCommLogger(), "MPI_Comm_Size failed");
    exit(EXIT_FAILURE);
  }

  if (MPI_Comm_rank(MPI_COMM_WORLD, &MPI_rank) != MPI_SUCCESS) {
    LOG4CXX_FATAL(getCommLogger(), "MPI_Comm_rank failed");
    exit(EXIT_FAILURE);
  }

  LOG4CXX_INFO_FMT(
      getCommLogger(),
      "MPI_rank: {}; MPI_sitesCount: {}", MPI_rank, MPI_sitesCount);

  getAlgoLayer()->getSessionLayer()->setRank(static_cast<rank_t>(MPI_rank));

  return MPI_sitesCount;
}

void Comm_MPI::openDestAndWaitIncomingMsg(std::vector<rank_t> const& dest,
                                        size_t nbAwaitedConnections) {
  // Wait until all tasks joined MPI
  MPI_Barrier(MPI_COMM_WORLD);

  getAlgoLayer()->callbackInitDone();

  process_Comm_MPI();

  MPI_Finalize();
}

void Comm_MPI::process_Comm_MPI() {
  auto task_to_receive_msg = async(launch::async, &Comm_MPI::receive_msg_loop, this);

  auto task_to_process_msg = async(launch::async, &Comm_MPI::process_msg_loop, this);

  task_to_receive_msg.get();
  LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Finished receive_msg_loop", getAlgoLayer()->getSessionLayer()->getRank());
  LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Waiting for process_msg_loop", getAlgoLayer()->getSessionLayer()->getRank());
  task_to_process_msg.get();
  LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Finished process_msg_loop", getAlgoLayer()->getSessionLayer()->getRank());
}

void Comm_MPI::receive_msg_loop() {
  while (!commTerminated) {
    LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Starting receive loop",
      getAlgoLayer()->getSessionLayer()->getRank());

    int msgSize;
    int probe_completed = 0;
    MPI_Status status;

    while (!probe_completed) {
      if (commTerminated) {
        break;
      }
      MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &probe_completed, &status);
    }
    if (commTerminated) {
      break;
    }
    MPI_Get_count(&status, MPI_BYTE, &msgSize);

    int const source = status.MPI_SOURCE;
    vector<char> buffer(msgSize);

    LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Probe and get count done",
      getAlgoLayer()->getSessionLayer()->getRank());

    MPI_Recv(buffer.data(), msgSize, MPI_BYTE, source, 0, MPI_COMM_WORLD, &status);

    LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Received message",
      getAlgoLayer()->getSessionLayer()->getRank());

    string msg(buffer.begin(), buffer.end());

    addMessageToQueue(msg);
    queue_cv.notify_one();
  }
}

void Comm_MPI::addMessageToQueue(string& msg) {
  lock_guard lock(MPI_mutex);
  message_queue.push(move(msg));
}

void Comm_MPI::process_msg_loop() {
  while (!commTerminated) {
    unique_lock lock(MPI_mutex);
    queue_cv.wait(lock, [this] { return !message_queue.empty() || commTerminated; });

    if (!message_queue.empty()) {
      string msg = move(message_queue.front());
      message_queue.pop();
      lock.unlock();

      getAlgoLayer()->callbackReceive(move(msg));
    }
  }
}

void Comm_MPI::multicastMsg(const std::string& algoMsgAsString) {
  int sitesCount;
  MPI_Comm_size(MPI_COMM_WORLD, &sitesCount);

  for (int i = 0; i < sitesCount; i++) {
    send(static_cast<rank_t>(i), algoMsgAsString);
  }
}

void Comm_MPI::send(rank_t const r, const std::string& algoMsgAsString) {
  auto const msgSize = static_cast<int>(algoMsgAsString.size());

  LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Waiting send to {:d}",
    getAlgoLayer()->getSessionLayer()->getRank(), r);

  MPI_Send(algoMsgAsString.data(), msgSize, MPI_BYTE, r, 0, MPI_COMM_WORLD);
  LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Message send to {:d}",
    getAlgoLayer()->getSessionLayer()->getRank(), r);
}

void Comm_MPI::terminate() {
  LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Calling terminate", getAlgoLayer()->getSessionLayer()->getRank());

  setCommTerminated();

  queue_cv.notify_all();

  LOG4CXX_INFO_FMT(getCommLogger(), "Rank #{:d}: Terminate finished", getAlgoLayer()->getSessionLayer()->getRank());
}

void Comm_MPI::setCommTerminated() {
  lock_guard lock(MPI_mutex);
  commTerminated = true;
}

[[nodiscard]] std::string Comm_MPI::toString() {
  return "Comm_MPI";
}

} // namespace fbae::core::CommLayer::Comm_MPI