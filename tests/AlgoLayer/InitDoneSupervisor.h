//
// Created by simatic on 05/04/202424.
//

#ifndef FBAE_INIT_DONE_SUPERVISOR_H
#define FBAE_INIT_DONE_SUPERVISOR_H

#include <semaphore>

/**
 * @brief Class interfacing AlgoStub instances and a CommLayerTest instance.
 */
class InitDoneSupervisor {
public:
    /**
     * @brief Method to be be called by an AlgoStub instance when its own @callback() method is called.
     */
    void callbackInitDone();

    /**
     * @brief Method called by a CommLayerTest instance to wait for an AlgoStub instance to have its own @callback() method called.
     */
    void waitInitDone();

private:
    static constexpr int initialWaitCallbackInitDone = 0;
    std::counting_semaphore<initialWaitCallbackInitDone> waitCallbackInitDone{initialWaitCallbackInitDone};
};


#endif //FBAE_INIT_DONE_SUPERVISOR_H
