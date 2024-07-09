//
// Created by simatic on 05/04/202424.
//

#ifndef FBAE_INIT_DONE_SUPERVISOR_H
#define FBAE_INIT_DONE_SUPERVISOR_H

#include <semaphore>

namespace fbae::core::AlgoLayer {

/**
 * @brief Class interfacing AlgoStub instances and a CommLayerTest instance.
 */
class InitDoneSupervisor {
 public:
  /**
   * @brief Method to be be called by an AlgoStub instance when its own
   * @callback() method is called.
   */
  void callbackInitDone();

  /**
   * @brief Method called by a CommLayerTest instance to wait for an AlgoStub
   * instance to have its own @callback() method called.
   */
  void waitInitDone();

 private:
  /**
   * @brief During tests, maximum number of @AlgoLayer instances in parallel.
   */
  static constexpr std::ptrdiff_t maximumNumberAlgoLayerInstancesDuringTests =
      8;
  static constexpr std::ptrdiff_t initialWaitCallbackInitDone = 0;
  std::counting_semaphore<maximumNumberAlgoLayerInstancesDuringTests>
      waitCallbackInitDone{initialWaitCallbackInitDone};
};

}  // namespace fbae::core::AlgoLayer

#endif  // FBAE_INIT_DONE_SUPERVISOR_H
