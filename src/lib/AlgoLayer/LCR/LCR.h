#pragma once

#include "../AlgoLayer.h"
#include "LCRMessage.h"

/** The LCR Algorithm layer. */
class LCRLayer: AlgoLayer {
public:
    explicit LCRLayer(std::unique_ptr<CommLayer> commLayer);
    void callbackReceive(std::string && algoMsgAsString) override;
    void execute() override;
    void terminate() override;
    void totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) override;
    std::string toString() override;
private:
    std::vector<uint32_t> vectorClock;
    std::vector<fbae_LCRAlgoLayer::StructBroadcastMessage> pending;
};

