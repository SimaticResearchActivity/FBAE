//
// Created by simatic on 2/17/24.
//

#ifndef FBAE_SESSION_STUB_H
#define FBAE_SESSION_STUB_H

#include "../../src/SessionLayer/SessionLayer.h"
#include "../../src/AlgoLayer/AlgoLayer.h"

class SessionStub : public SessionLayer {
public:
    SessionStub(const Arguments &arguments, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer);

    void callbackDeliver(rank_t senderPos, std::string && msg) override;
    void callbackInitDone() const override;
    void execute() override;
    std::vector<std::pair<rank_t, std::string>> &getDelivered();
private:
    /**
     * @brief Vector of [sender,msg] for which @callbackDeliver() method was called
     */
    std::vector<std::pair<rank_t, std::string>> delivered;
};


#endif //FBAE_SESSION_STUB_H
