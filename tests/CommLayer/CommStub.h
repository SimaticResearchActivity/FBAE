//
// Created by simatic on 2/17/24.
//

#ifndef FBAE_COMM_STUB_H
#define FBAE_COMM_STUB_H
#include "../../src/CommLayer/CommLayer.h"

class CommStub : public CommLayer {
public:
    std::vector<rank_t> &getConnectedDest();
    std::vector<std::pair<rank_t, std::string>> &getSent();
    void multicastMsg(const std::string &msg) override;
    void openDestAndWaitIncomingMsg(std::vector<rank_t> const & dest, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) override;
    void send(rank_t r, const std::string &msg) override;
    void terminate() override;
    std::string toString() override;
private:
    /**
     * @brief Connected destinations during execution of @openDestAndWaitIncomingMsg() method
     */
    std::vector<rank_t> connectedDest;

    /**
     * @brief List of [destination,msg] sent by @send() or @multicastMsg() methods
     */
    std::vector<std::pair<rank_t, std::string>> sent;

};


#endif //FBAE_COMM_STUB_H
