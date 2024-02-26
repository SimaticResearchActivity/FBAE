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

    void callbackDeliver(rank_t senderPos, fbaeSL::SessionMsg msg) override;
    void callbackInitDone() override;
    void execute() override;
    [[nodiscard]] std::vector<std::pair<rank_t, fbaeSL::SessionMsg>> & getDelivered();
    [[nodiscard]] bool isCallbackInitDoneCalled() const;
private:
    /**
     * @brief Vector of [sender,msg] for which @callbackDeliver() method was called
     */
    std::vector<std::pair<rank_t, fbaeSL::SessionMsg>> delivered;

    /**
     * @brief Indicates whether @SessionLayer::callbackInitDone called or not.
     */
    bool callbackInitDoneCalled{false};
};


#endif //FBAE_SESSION_STUB_H
