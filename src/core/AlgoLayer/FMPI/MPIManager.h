#include <mpi.h>

#include <mutex>
#include <stdexcept>

#include "../../Logger/LoggerConfig.h"

namespace fbae::core::AlgoLayer::FMPI {
  
class MPIManager {
 public:
  MPIManager() = default;
  ~MPIManager() { MPI_finalize(); }

  static MPIManager& getInstance() {
    static MPIManager instance;
    return instance;
  }


  void MPI_initialize(Logger::LoggerPtr logger = nullptr) {
    std::scoped_lock<std::mutex> lock(mutex_);

    if (!initialized) {
      if (int provided; MPI_Init_thread(nullptr, nullptr, MPI_THREAD_MULTIPLE, &provided) != MPI_SUCCESS) {
        LOG4CXX_FATAL(logger, "Failed to initialize MPI");
        exit(EXIT_FAILURE);
      }
      initialized = true;
    }
  }

  void MPI_finalize() {
    std::scoped_lock<std::mutex> lock(mutex_);

    if (initialized) {
      MPI_Finalize();
      initialized = false;
    }
  }

 private:
  bool initialized = false;
  std::mutex mutex_;
};

}  // namespace fbae::core::AlgoLayer::FMPI