//
// Created by simatic on 2/10/24.
//

#ifndef FBAE_SESSION_H
#define FBAE_SESSION_H

#include <future>
#include <memory>
#include "../SessionLayer.h"
#include "../../Measures.h"
#include "../../AlgoLayer/AlgoLayer.h"

class Session : public SessionLayer {
public:
    Session(const Arguments &arguments, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer, std::unique_ptr<CommLayer> commLayer);

    void callbackDeliver(rank_t senderPos, std::string && msg) override;
    void callbackInitDone() const override;
    void execute() override;
    [[nodiscard]] CommLayer *getCommLayer() const override;
    [[nodiscard]] rank_t getRankFromRuntimeArgument() const override;

private:
    const rank_t rank;
    std::unique_ptr<AlgoLayer> algoLayer;
    std::unique_ptr<CommLayer> commLayer;
    Measures measures;
    int32_t numPerfMeasure{0};
    int32_t nbReceivedPerfResponseForSelf{0};
    size_t nbReceivedFirstBroadcast{0};
    size_t nbReceivedFinishedPerfMeasures{0};
    std::future<void> taskSendPeriodicPerfMessage;

    /**
     * @brief Broadcasts a @PerfMeasure message with @msgNum incremented by 1.
     */
    void broadcastPerfMeasure();

    /**
     * @brief Called by @callbackDeliver to process @FinishedPerfMeasures message
     * @param senderPos Rank of message sender.
     */
    void processFinishedPerfMeasuresMsg(rank_t senderPos);

    /**
     * @brief Called by @callbackDeliver to process @FirstBroadcast message
     * @param senderPos Rank of message sender.
     */
    void processFirstBroadcastMsg(rank_t senderPos);

    /**
     * @brief Called by @callbackDeliver to process @PerfMeasure message
     * @param senderPos Rank of message sender.
     * @param msg Message to process.
     */
    void processPerfMeasureMsg(rank_t senderPos, std::string && msg);

    /**
     * @brief Called by @callbackDeliver to process @PerfMeasure message
     * @param senderPos Rank of message sender.
     * @param msg Message to process.
     */
    void processPerfResponseMsg(rank_t senderPos, std::string && msg);

    /**
     * @brief Thread to send PerfMessage at @Param::frequency per second.
     */
    void sendPeriodicPerfMessage();
};


#endif //FBAE_SESSION_H
