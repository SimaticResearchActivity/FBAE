//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBBALGOLAYER_H
#define FBAE_BBOBBALGOLAYER_H


#include "AlgoLayer.h"

class BBOBBAlgoLayer : public AlgoLayer {
private :
    std::vector<HostTuple> broadcasters; //Every broadcaster needs the list of all broadcasters
public :
    void callbackHandleMessageAsHost(std::unique_ptr<CommPeer> peer, const std::string &msgString) override;
    void callbackHandleMessageAsNonHostPeer(std::unique_ptr<CommPeer> peer, const std::string &msgString) override;
    bool executeAndProducedStatistics() override;
    void totalOrderBroadcast(const std::string &msg) override;
    void terminate() override;
    std::string toString() override;
};


#endif //FBAE_BBOBBALGOLAYER_H
