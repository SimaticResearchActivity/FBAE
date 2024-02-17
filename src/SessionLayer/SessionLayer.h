#pragma once

#include "../basicTypes.h"
#include "../CommLayer/CommLayer.h"
#include "../Arguments.h"
class AlgoLayer;

class SessionLayer {
public:
    virtual ~SessionLayer() = default;

    explicit SessionLayer(const Arguments &aArguments,std::unique_ptr<AlgoLayer> anAlgoLayer);

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is able to deliver totalOrderBroadcast @msg.
     * @param senderPos Position of @msg sender in @AlgoLayer::broadcasters.
     * @param seqNum Sequence number of @msg.
     * @param msg Message to be delivered.
     */
    virtual void callbackDeliver(rank_t senderPos, std::string && msg) = 0;

    /**
     * @brief Callback called by @AlgoLayer when @AlgoLayer is initialized locally.
     */
    virtual void callbackInitDone() const = 0;

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
    [[nodiscard]] virtual rank_t getRankFromRuntimeArgument() const = 0;

private:
    const Arguments &arguments;
    std::unique_ptr<AlgoLayer> algoLayer;
};
