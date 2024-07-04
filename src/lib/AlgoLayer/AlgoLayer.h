#pragma once

#include <condition_variable>
#include <thread>
#include <memory>
#include <optional>
#include "../Logger/LoggerConfig.h"
#include "../Arguments.h"
#include "../CommLayer/CommLayer.h"
#include "AlgoLayerMsg.h"

class SessionLayer;

class AlgoLayer {
public:
    virtual ~AlgoLayer() = default;

    explicit AlgoLayer(std::unique_ptr<CommLayer> commLayer, std::string const& logger_name);

    /**
     * @brief Returns @batchWaitingSessionMsg encapsulated in a @BatchSessionMsg
     * @param senderPos position of sender of @batchWaitingSessionMsg
     * @return @batchWaitingSessionMsg encapsulated in a @BatchSessionMsg
     */
    [[nodiscard]] std::optional<fbae_AlgoLayer::BatchSessionMsg> batchGetBatchMsgs(rank_t senderPos);

    /**
     * @brief Callback to be called by @AlgoLayer when @AlgoLayer is using @batchWaitingSessionMsg.
     * @param senderPos Position of @msg sender in @AlgoLayer::broadcasters.
     * @param msg Message to be delivered.
     */
    void batchNoDeadlockCallbackDeliver(rank_t senderPos, std::shared_ptr<fbae_SessionLayer::SessionBaseClass> const& msg);

    /**
     * @brief Register that current thread accepts that it is not concerned by value of @batchCtrlShortcut
     * (it will always wait for @batchWaitingSessionMsg to be small enough; See Issue #37).
     */
    void batchRegisterThreadForFullBatchCtrl();

    /**
     * @brief Handles message received by @CommLayer
     * @param algoMsgAsString String containing message.
     */
    virtual void callbackReceive(std::string && algoMsgAsString) = 0;

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
     * @brief Broadcasts message in a total-order manner. Note: If this method is not override, the message is stored in
     * @batchWaitingSessionMsg
     * @param sessionMsg Message to totally-order totalOrderBroadcast.
     */
    virtual void totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg);

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

protected :
    /**
    * @brief Return the logger of the parent
    */
    [[nodiscard]] fbae::LoggerPtr getAlgoLogger() const;

private:
    std::optional<fbae_AlgoLayer::BatchSessionMsg> batchGetBatchMsgsWithLock(rank_t senderPos);

    /**
     * @brief Condition variable coupled with @batchCtrlMtx to control that batch of messages in
    * @batchWaitingSessionMsg is not too big
    */
    std::condition_variable batchCtrlCondVar;

    /**
     * @brief Mutex coupled with @batchCtrlCondVar to control that batch of messages in
    * @batchWaitingSessionMsg is not too big
     */
    std::mutex batchCtrlMtx;

    /**
     * @brief Variable used to shortcut BatchCtrl mechanism (with @batchCtrlMtx and @batchCtrlCondVar), so that, in
    * order to avoid deadlocks, we accept that the number of bytes stored in @batchWaitingSessionMsg is greater
    * than @maxBatchSize of @Arguments instance.
    */
    bool batchCtrlShortcut{false};

    /**
     * @brief Vector of threads which registered to accept that they are not concerned by value of @batchCtrlShortcut
     * (they always wait for @batchWaitingSessionMsg to be small enough; See Issue #37).
     */
    std::vector<std::thread::id> batchCtrlThreadsRegisteredForFullBatchCtrl{};

    /**
     * @brief PerfMeasures messages waiting to be broadcast.
     */
    std::vector<fbae_SessionLayer::SessionMsg> batchWaitingSessionMsg;

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

    /**
     * @brief Logger used to print informations
     */
    fbae::LoggerPtr m_logger;
};
