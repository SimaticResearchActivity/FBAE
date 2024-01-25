//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBBALGOLAYER_H
#define FBAE_BBOBBALGOLAYER_H

#include <latch>
#include <map>

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "../AlgoLayer.h"
#include "BBOBBAlgoLayerMsg.h"

class BBOBBAlgoLayer : public AlgoLayer {
private :
    std::vector<std::unique_ptr<CommPeer>> peers; //peers that the entity will communicate with
    std::vector<int> peersRank;
    /**
     * @brief Latch used to guarantee that all outgoing connections are established (and thus peers and peersRank are
     * initialized and ready to be used).
     */
    std::latch peers_peersRank_ready{1};

    bool sendWave = true;

    int seqNum = 0;
    int nbConnectedBroadcasters= 0;

    /**
     * @brief The number of steps in each wave is also the number of peers this peer will connect to and also the
     * number of peers which will connect to this peer.
     */
    int nbStepsInWave;

    std::vector<std::string> msgWaitingToBeBroadcast;
    fbae_BBOBBAlgoLayer::StepMsg lastSentStepMsg;
    std::map<int, fbae_BBOBBAlgoLayer::StepMsg> currentWaveReceivedStepMsg;
    std::map<int, fbae_BBOBBAlgoLayer::StepMsg> nextWaveReceivedStepMsg;

public :
    bool callbackHandleMessage(std::unique_ptr<CommPeer> peer, const std::string &msgString) override;
    bool executeAndProducedStatistics() override;
    void totalOrderBroadcast(const std::string &msg) override;
    void terminate() override;
    std::string toString() override;
    void beginWave();
    void CatchUpIfLateInMessageSending();
};



#endif //FBAE_BBOBBALGOLAYER_H
