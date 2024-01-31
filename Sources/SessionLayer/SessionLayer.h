#pragma once

#include <latch>
#include <memory>
#include "../AlgoLayer/AlgoLayer.h"
#include "../basicTypes.h"
#include "../Measures.h"
#include "../Param.h"

class SessionLayer {
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
    void processPerfMeasureMsg(rank_t senderRank, const std::string &msg);

    /**
     * @brief Called by @callbackDeliver to process @PerfMeasure message
     * @param senderRank Rank of message sender.
     * @param msg Message to process.
     */
    void processPerfResponseMsg(rank_t senderRank, const std::string &msg);

    /**
     * @brief Thread to send PerfMessage at @Param::frequency per second.
     */
    void sendPeriodicPerfMessage();

public:
    SessionLayer(const Param &param, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer, std::unique_ptr<CommLayer> commLayer);

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is able to deliver totalOrderBroadcast @msg.
     * @param senderRank Rank of @msg sender.
     * @param seqNum Sequence number of @msg.
     * @param msg Message to be delivered.
     */
    void callbackDeliver(rank_t senderRank, const std::string &msg);

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is initialized locally.
     */
    void callbackInitDone() const;

    /**
     * @brief Entry point of @SessionLayer to executeAndProducedStatistics it.
     */
    void execute();

    /**
     * @brief Getter for @commlayer.
     * @return @commlayer
     */
    [[nodiscard]] CommLayer *getCommLayer() const;

    /**
     * @brief Getter for @param.
     * @return @param
     */
    [[nodiscard]] const Param &getParam() const;

    /**
     * @brief Getter for @rank.
     * @return @rank.
     */
    [[nodiscard]] rank_t getRank() const;
};
