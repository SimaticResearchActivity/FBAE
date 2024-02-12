//
// Created by simatic on 2/10/24.
//

#ifndef FBAE_SESSION_H
#define FBAE_SESSION_H

#include <latch>
#include <memory>
#include "../SessionLayer.h"
#include "../../Measures.h"
#include "../../AlgoLayer/AlgoLayer.h"

class Session : public SessionLayer {
public:
    Session(const Param &param, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer, std::unique_ptr<CommLayer> commLayer);

    void callbackDeliver(rank_t senderRank, std::string && msg) override;
    void callbackInitDone() const override;
    void execute() override;
    [[nodiscard]] CommLayer *getCommLayer() const override;
    [[nodiscard]] const Param &getParam() const override;
    [[nodiscard]] rank_t getRankFromRuntimeArgument() const override;

private:
    const Param &param;
    const rank_t rank;
    std::unique_ptr<AlgoLayer> algoLayer;
    std::unique_ptr<CommLayer> commLayer;
    Measures measures;
    int32_t numPerfMeasure{0};
    int32_t nbReceivedPerfResponseForSelf{0};
    size_t nbReceivedFirstBroadcast{0};
    size_t nbReceivedFinishedPerfMeasures{0};
    std::latch okToSendPeriodicPerfMessage{1};

    /**
     * @brief Broadcasts a @PerfMeasure message with @msgNum incremented by 1.
     */
    void broadcastPerfMeasure();

    /**
     * @brief Called by @callbackDeliver to process @FinishedPerfMeasures message
     * @param senderRank Rank of message sender.
     */
    void processFinishedPerfMeasuresMsg(rank_t senderRank);

    /**
     * @brief Called by @callbackDeliver to process @FirstBroadcast message
     * @param senderRank Rank of message sender.
     */
    void processFirstBroadcastMsg(rank_t senderRank);

    /**
     * @brief Called by @callbackDeliver to process @PerfMeasure message
     * @param senderRank Rank of message sender.
     * @param msg Message to process.
     */
    void processPerfMeasureMsg(rank_t senderRank, std::string && msg);

    /**
     * @brief Called by @callbackDeliver to process @PerfMeasure message
     * @param senderRank Rank of message sender.
     * @param msg Message to process.
     */
    void processPerfResponseMsg(rank_t senderRank, std::string && msg);

    /**
     * @brief Thread to send PerfMessage at @Param::frequency per second.
     */
    void sendPeriodicPerfMessage();
};


#endif //FBAE_SESSION_H
