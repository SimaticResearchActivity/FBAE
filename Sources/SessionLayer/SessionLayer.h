#pragma once

#include <memory>
#include "../AlgoLayer/AlgoLayer.h"
#include "../Measures.h"
#include "../Param.h"

class SessionLayer {
private:
    const Param &param;
    const int rank;
    std::unique_ptr<AlgoLayer> algoLayer;
    std::unique_ptr<CommLayer> commLayer;
    Measures measures;
public:
    SessionLayer(const Param &param, int rank, std::unique_ptr<AlgoLayer> algoLayer, std::unique_ptr<CommLayer> commLayer);

    /**
     * @brief Broadcasts a @PerfMeasure message with @msgNum incremented by 1.
     * @param msgNum Value of msgNum to increment before storing in @PerfMeasure message
     * @return Incremented value of @msgNum
     */
    unsigned int broadcastPerfMeasure(unsigned int msgNum);

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is able to deliver totalOrderBroadcast @msg.
     * @param senderRank Rank of @msg sender.
     * @param seqNum Sequence number of @msg.
     * @param msg Message to be delivered.
     */
    void callbackDeliver(int senderRank, int seqNum, const std::string &msg);

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
    [[nodiscard]] int getRank() const;

    /**
     * @brief Called by @callbackDeliver to process @FinishedPerfMeasures message
     * @param senderRank Rank of message sender.
     * @param seqNum Sequence number of message.
     */
    void processFinishedPerfMeasuresMsg(int senderRank, int seqNum);

    /**
     * @brief Called by @callbackDeliver to process @FirstBroadcast message
     * @param senderRank Rank of message sender.
     * @param seqNum Sequence number of message.
     * @param msgNum Current message number
     * @return New value of @msgNum
     */
    [[nodiscard]] unsigned int processFirstBroadcastMsg(int senderRank, int seqNum, unsigned int msgNum);

    /**
     * @brief Called by @callbackDeliver to process @PerfMeasure message
     * @param senderRank Rank of message sender.
     * @param seqNum Sequence number of message.
     * @param msg Message to process.
     */
    void processPerfMeasureMsg(int senderRank, int seqNum, const std::string &msg);

    /**
     * @brief Called by @callbackDeliver to process @PerfMeasure message
     * @param senderRank Rank of message sender.
     * @param seqNum Sequence number of message.
     * @param msgNum Current message number
     * @param msg Message to process.
     * @return New value of @msgNum
     */
    [[nodiscard]] unsigned int processPerfResponseMsg(int senderRank, int seqNum, unsigned int msgNum, const std::string &msg);
};
