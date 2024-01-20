//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBBALGOLAYER_H
#define FBAE_BBOBBALGOLAYER_H

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "../AlgoLayer.h"

class BBOBBAlgoLayer : public AlgoLayer {
private :
    std::vector<std::unique_ptr<CommPeer>> peers; //peers that the entity will communicate with
    std::vector<int> peersRank;
    int currentWave = 0;
    int sendWave = 1;

    std::vector<std::string> msgWaitingToBeBroadcasted;
    std::vector<std::vector<std::string>> messagesOfOneWave;
    std::vector<bool> alreadySent;
    std::vector<bool> received;

    std::vector<std::vector<std::string>> messagesOfNextWave;
    std::vector<bool> receivedNextWave;

public :
    bool callbackHandleMessage(std::unique_ptr<CommPeer> peer, const std::string &msgString) override;
    bool executeAndProducedStatistics() override;
    void totalOrderBroadcast(const std::string &msg) override;
    void terminate() override;
    std::string toString() override;
    void beginWave(int wave);
    bool sendStepMessages();
};



#endif //FBAE_BBOBBALGOLAYER_H
