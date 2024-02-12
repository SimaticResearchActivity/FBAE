#pragma once

#include "../basicTypes.h"
#include "../CommLayer/CommLayer.h"
#include "../Param.h"


class SessionLayer {
public:
    virtual ~SessionLayer() = default;

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is able to deliver totalOrderBroadcast @msg.
     * @param senderRank Rank of @msg sender.
     * @param seqNum Sequence number of @msg.
     * @param msg Message to be delivered.
     */
    virtual void callbackDeliver(rank_t senderRank, std::string && msg) = 0;

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is initialized locally.
     */
    virtual void callbackInitDone() const = 0;

    /**
     * @brief Entry point of @SessionLayer to execute it.
    */
    virtual void execute() = 0;

    /**
     * @brief Getter for @commlayer.
    * @return @commlayer
    */
    [[nodiscard]] virtual CommLayer *getCommLayer() const = 0;

    /**
     * @brief Getter for @param.
     * @return @param
     */
    [[nodiscard]] virtual const Param &getParam() const = 0;

    /**
     * @brief Getter for @rank.
     * @return @rank.
     */
    [[nodiscard]] virtual rank_t getRankFromRuntimeArgument() const = 0;
};
