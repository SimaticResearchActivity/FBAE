#pragma once

#include "../basicTypes.h"
#include "../Arguments.h"
#include "../CommLayer/CommLayer.h"
#include "SessionLayerMsg.h"
class AlgoLayer;

class SessionLayer {
public:
    virtual ~SessionLayer() = default;

    explicit SessionLayer(const Arguments &arguments, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer);

    /**
     * @brief Constructor used only for tests.
     * @param rank
     */
    explicit SessionLayer(const Arguments &arguments, rank_t rank);

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is able to deliver totalOrderBroadcast @msg.
     * @param senderPos Position of @msg sender in @AlgoLayer::broadcasters.
     * @param seqNum Sequence number of @msg.
     * @param msg Message to be delivered.
     */
    virtual void callbackDeliver(rank_t senderPos, fbae_SessionLayer::SessionMsg msg) = 0;

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is initialized locally.
     */
    virtual void callbackInitDone() = 0;

    /**
     * @brief Entry point of @SessionLayer to execute it.
    */
    virtual void execute() = 0;

    /**
     * @brief Getter for @algolayer.
    * @return @algolayer
    */
    [[nodiscard]] AlgoLayer * getAlgoLayer() const;

    /**
     * @brief Getter for @arguments.
     * @return @arguments
     */
    [[nodiscard]] const Arguments &getArguments() const;

    /**
     * @brief Getter for @rank.
     * @return @rank.
     */
    [[nodiscard]] virtual rank_t getRank() const;

private:
    const Arguments &arguments;
    std::unique_ptr<AlgoLayer> algoLayer;
    const rank_t rank;
};
