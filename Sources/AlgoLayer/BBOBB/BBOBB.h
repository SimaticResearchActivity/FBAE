//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBB_H
#define FBAE_BBOBB_H

#include <condition_variable>
#include <map>

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "../AlgoLayer.h"
#include "BBOBBMsg.h"

class BBOBB : public AlgoLayer {
public :
    void callbackHandleMessage(std::string && msgString) override;
    void callbackInitDone() override;
    void execute() override;
    void totalOrderBroadcast(std::string && msg) override;
    void terminate() override;
    std::string toString() override;
    void beginWave();
    void catchUpIfLateInMessageSending();
private :
    /**
     * @brief Vectors of rank of outgoing peers
     */
    std::vector<rank_t> peersRank;

    /**
     * @brief Mutex coupled with @condVarBatchCtrl to control that batch of messages in msgsWaitingToBeBroadcast is not
     * too big
     */
    std::mutex mtxBatchCtrl;

    /**
     * @brief Condition variable coupled with @mtxBatchCtrl to control that batch of messages in msgsWaitingToBeBroadcast is not
     * too big
     */
    std::condition_variable condVarBatchCtrl;

    /**
     * @brief Variable used to shortcut BatchCtrl mechanism (with @mtxBatchCtrl and @condVarBatchCtrl), so that, in
     * order to avoid deadlocks, we accept that the number of bytes stored in @msgsWaitingToBeBroadcast is greater than
     * @maxBatchSize of @Param instance.
     */
    bool shortcutBatchCtrl{false};

    /**
     * @brief True when @disconnect() method has been called.
     */
    bool algoTerminated{false};

    /**
     * @brief The number of steps in each wave is also the number of peers this peer will connect to and also the
     * number of peers which will connect to this peer.
     */
    int nbStepsInWave{0};

    std::vector<std::string> msgsWaitingToBeBroadcast;
    fbae_BBOBBAlgoLayer::StepMsg lastSentStepMsg;
    std::map<int, fbae_BBOBBAlgoLayer::StepMsg> currentWaveReceivedStepMsg;
    std::map<int, fbae_BBOBBAlgoLayer::StepMsg> nextWaveReceivedStepMsg;

};
#endif //FBAE_BBOBB_H
