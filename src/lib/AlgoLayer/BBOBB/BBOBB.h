//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBB_H
#define FBAE_BBOBB_H

#include <map>

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "../AlgoLayer.h"
#include "BBOBBMsg.h"

class BBOBB : public AlgoLayer {
public :
    explicit BBOBB(std::unique_ptr<CommLayer> commLayer);
    void callbackReceive(std::string && algoMsgAsString) override;
    void callbackInitDone() override;
    void execute() override;
    void terminate() override;
    std::string toString() override;
private :
    /**
     * @brief Method called when a wave begins
     */
    void beginWave();

    /**
     * @brief Method called to see if current participant is late in processing received step messages.
     */
    void catchUpIfLateInMessageSending();

    /**
     * @brief True when @disconnect() method has been called.
     */
    bool algoTerminated{false};

    /**
     * @brief Received @Step messages which belong to current wave.
     */
    std::map<int, fbae_BBOBBAlgoLayer::StepMsg> currentWaveReceivedStepMsg;

    /**
     * @brief Delivers all SessionMsg contained in the different batches of messages received
     */
    void deliverBatchSessionMsg();

    /**
     * @brief Last @Step message which has been sent.
     */
    fbae_BBOBBAlgoLayer::StepMsg lastSentStepMsg;

    /**
     * @brief The number of steps in each wave is also the number of peers this peer will connect to and also the
     * number of peers which will connect to this peer.
     */
    int nbStepsInWave{0};

    /**
     * @brief Received @Step messages which belong to next wave.
     */
    std::map<int, fbae_BBOBBAlgoLayer::StepMsg> nextWaveReceivedStepMsg;

    /**
     * @brief Vectors of positions of outgoing peers
     */
    std::vector<rank_t> peersPos;

    /**
     * @brief Process Step message contained in @algoMsgAsString
     * @param algoMsgAsString
     */
    void processStepMsg(std::string && algoMsgAsString);
};
#endif //FBAE_BBOBB_H
