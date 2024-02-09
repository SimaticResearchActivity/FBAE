#pragma once

#include "memory"
#include "../CommLayer/CommLayer.h"
#include "../Param.h"
class SessionLayer;

class AlgoLayer {
private:
    /**
     * @brief List of @sites which are indeed doing broadcasts.
     */
    std::vector<HostTuple> broadcasters;
    SessionLayer *session{nullptr};

public:
    virtual ~AlgoLayer() = default;

    /**
     * @brief Handles message received from @an incoming peer.
     * @param msgString String containing message.
     */
    virtual void callbackHandleMessage(std::string && msgString) = 0;

    /**
     * @brief Executes concrete totalOrderBroadcast algorithm. Returns when algorithm is done.
     * @return true if the execution lead to the production of statistics (e.g. case of a broadcaster in Sequencer
     * algorithm) and false otherwise (e.g. which can be the case of the sequencer in Sequencer algorithm of the
     * sequencer is not a broadcaster)
     */
    virtual bool executeAndCheckIfProducedStatistics() = 0;

    /**
     * @brief Getter for @broadcasters.
     * @return @broadcasters.
     */
    [[nodiscard]] const std::vector<HostTuple> &getBroadcasters() const;

    /**
     * @brief Getter for @session
     * @return @session
     */
    [[nodiscard]] SessionLayer *getSession() const;

    /**
     * @brief Setter for @broadcasters.
     * @param aBroadcasters  The value to set @broadcasters to.
     */
    void setBroadcasters(const std::vector<HostTuple> &aBroadcasters);

    /**
     * @brief Setter for @session
     * @param aSession
     */
    void setSession(SessionLayer *aSession);

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
};
