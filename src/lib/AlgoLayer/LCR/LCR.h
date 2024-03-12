#pragma once

#include "../AlgoLayer.h"
#include "LCRMessage.h"

/** @brief The LCR Algorithm layer. */
class LCRLayer: AlgoLayer {
public:
    explicit LCRLayer(std::unique_ptr<CommLayer> commLayer);

    void callbackReceive(std::string && algoMsgAsString) override;
    void execute() override;
    void terminate() override;
    void totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) override;

    std::string toString() override;

private:

    /**
     * @brief Internal function for handling reception of a message from another site.
     * @param message: The message we received that want to handle.
     * @return Optionally returns the message to send to the successor in the ring of sites.
    */
    std::optional<fbae_LCRAlgoLayer::StructBroadcastMessage> handleMessageReceive(
            fbae_LCRAlgoLayer::StructBroadcastMessage message);

    /**
     * @brief Internal function for handling reception of an acknowledgement from another site.
     * @param message: The acknowledgment message we received that want to handle.
     * @return Optionally returns the acknowledgement message to send to the successor in the ring of sites.
    */
    std::optional<fbae_LCRAlgoLayer::StructBroadcastMessage> handleAcknowledgmentReceive(
            fbae_LCRAlgoLayer::StructBroadcastMessage message);

    /**
     * @brief Internal function for handling trying to deliver the message to the particular site.
    */
    void tryDeliver();

    /**
     * @brief The internal vector clock of the site.
    */
    std::vector<uint32_t> vectorClock;

    /**
     * @brief The pending list of messages of the site.
    */
    std::vector<fbae_LCRAlgoLayer::StructBroadcastMessage> pending;
};
