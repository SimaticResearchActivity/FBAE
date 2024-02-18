#pragma once

#include <memory>
#include <optional>
#include "../CommLayer/CommLayer.h"
#include "../Arguments.h"
class SessionLayer;

class AlgoLayer {
public:
    virtual ~AlgoLayer() = default;

    explicit AlgoLayer(std::unique_ptr<CommLayer> commLayer);

    /**
     * @brief Handles message received by @CommLayer
     * @param msgString String containing message.
     */
    virtual void callbackReceive(std::string && msgString) = 0;

    /**
     * @brief Callback called by @CommLayer when @CommLayer is initialized locally.
     */
    virtual void callbackInitDone();

    /**
     * @brief Executes concrete Total Order Broadcast algorithm. Returns when algorithm is done.
     */
    virtual void execute() = 0;

    /**
     * @brief Getter for @broadcasters.
     * @return @broadcasters.
     */
    [[nodiscard]] const std::vector<rank_t> & getBroadcastersGroup() const;

    /**
     * @brief Getter for @commlayer.
    * @return @commlayer
    */
    [[nodiscard]] CommLayer *getCommLayer() const;

    /**
     * @brief Returns position of a broadcasting participant in @broadcasters vector.
     * @return Position in @broadcasters vector if participant is indeed a broadcasting participant and nullopt if participant is not a broadcasting participant.
     */
    [[nodiscard]] std::optional<rank_t> getPosInBroadcastersGroup() const;

    /**
     * @brief Getter for @sessionLayer
     * @return @sessionLayer
     */
    [[nodiscard]] SessionLayer *getSessionLayer() const;

    /**
     * @brief Indicates whether @AlgoLayer is broadcasting messages or not.
     * @return true if the execution of @ALgoLayer lead to the production of statistics (e.g. case of a broadcaster in
     * Sequencer algorithm) and false otherwise (e.g. which can be the case of the sequencer in Sequencer algorithm of the
     * sequencer is not a broadcaster)
     */
     [[nodiscard]] bool isBroadcastingMessages() const;

    /**
     * @brief Setter for @broadcasters.
     * @param aBroadcastersGroup  The value to set @broadcasters to.
     */
    void setBroadcastersGroup(std::vector<rank_t> &&aBroadcastersGroup);

    /**
     * @brief Setter for @sessionLayer
     * @param aSessionLayer
     */
    void setSessionLayer(SessionLayer *aSessionLayer);

    /**
     * @brief Broadcasts @msg in a total-order manner.
     * @param msg Message to totally-order totalOrderBroadcast.
     */
    virtual void totalOrderBroadcast(std::string && msg) = 0;

    /**
     * @brief Terminates execution of concrete totalOrderBroadcast algorithm. Eventually this call will lead to the
     * return of @execute method.
     */
    virtual void terminate() = 0;

    /**
     * @brief Return the name of the algorithm used as @AlgoLayer
     * @return Name of the algorithm used as @AlgoLayer
     */
    [[nodiscard]] virtual std::string toString() = 0;

private:
    /**
     * @brief Rank of @sites which are indeed doing broadcasts.
     */
    std::vector<rank_t> broadcastersGroup;

    /**
     * @brief @CommLayer used by this @AlgoLayer
     */
    std::unique_ptr<CommLayer> commLayer;

    /**
     * @brief @SessionLayer which uses this @AlgoLayer
     */
    SessionLayer *sessionLayer{nullptr};
};
