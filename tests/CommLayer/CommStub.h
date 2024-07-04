//
// Created by simatic on 2/17/24.
//

#ifndef FBAE_COMM_STUB_H
#define FBAE_COMM_STUB_H
#include "CommLayer/CommLayer.h"

class CommStub : public CommLayer {
public:
    explicit CommStub();
    [[nodiscard]] std::vector<rank_t> &getConnectedDest();
    [[nodiscard]] size_t getNbAwaitedConnections() const;
    [[nodiscard]] std::vector<std::pair<rank_t, std::string>> &getSent();
    void multicastMsg(const std::string &algoMsgAsString) override;
    void openDestAndWaitIncomingMsg(std::vector<rank_t> const & dest, size_t aNbAwaitedConnections, AlgoLayer *aAlgoLayer) override;
    void send(rank_t r, const std::string &algoMsgAsString) override;
    void terminate() override;
    std::string toString() override;
private:
    /**
     * @brief Connected destinations during execution of @openDestAndWaitIncomingMsg() method
     */
    std::vector<rank_t> connectedDest;

    /**
     * @brief Number of awaited connections
     */
    size_t nbAwaitedConnections;

    /**
     * @brief List of [destination,msg] sent by @send() or @multicastMsg() methods
     */
    std::vector<std::pair<rank_t, std::string>> sent;

};


#endif //FBAE_COMM_STUB_H
