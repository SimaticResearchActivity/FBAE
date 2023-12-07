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
    std::vector<std::string> msgWaitingToBeBroadcasted;
    std::vector<std::vector<std::string>> messagesOfOneWave;
    std::vector<bool> alreadySent;
    int currentWave = 0;
    int seqNum{0};
public :
    bool callbackHandleMessage(std::unique_ptr<CommPeer> peer, const std::string &msgString) override;
    bool executeAndProducedStatistics() override;
    void totalOrderBroadcast(const std::string &msg) override;
    void terminate() override;
    std::string toString() override;
};



#endif //FBAE_BBOBBALGOLAYER_H
