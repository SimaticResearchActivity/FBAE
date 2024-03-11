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

    /** Internal function for handling when we receive a message from another site. */
    std::optional<fbae_LCRAlgoLayer::StructBroadcastMessage> handleMessageReceive(
            fbae_LCRAlgoLayer::StructBroadcastMessage message);

    /** Internal function for handling when we receive an acknowledgement from another site. */
    std::optional<fbae_LCRAlgoLayer::StructBroadcastMessage> handleAcknowledgmentReceive(
            fbae_LCRAlgoLayer::StructBroadcastMessage message);

    /** Internal function for handling trying to deliver the message to the particular site. */
    void tryDeliver(void);

    /** The internal vector clock of the site. */
    std::vector<uint32_t> vectorClock;

    /** The pending list of messages of the site. */
    std::vector<fbae_LCRAlgoLayer::StructBroadcastMessage> pending;
};

