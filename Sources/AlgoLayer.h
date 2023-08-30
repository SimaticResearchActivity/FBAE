#pragma once

#include <memory>
#include "CommLayer.h"
#include "CommPeer.h"
#include "Param.h"
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
     * @brief Executes concrete totalOrderBroadcast algorithm. Returns when algorithm is done.
     * @return true if the execution lead to the production of statistics (e.g. case of a broadcaster in Sequencer
     * algorithm) and false otherwise (e.g. case of the sequencer in Sequencer algorithm)
     */
    virtual bool executeAndProducedStatistics() = 0;

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
     * @brief Handles packet (stored in @packetString) received from @peer. Should be called when process is a host,
     * that is is has called @initHost() on @CommLayer.
     * @param peer Peer from which packet was received.
     * @param msgString String containing message.
     */
    virtual void handleMessageAsHost(std::unique_ptr<CommPeer> peer, const std::string &msgString) = 0;

    /**
     * @brief Handles packet (stored in @packetString) received from @peer. Should be called when process is just a
     * peer and not a host, that is has /never/ called @initHost() on @CommLayer.
     * @param peer Peer from which packet was received.
     * @param msgString String containing message.
     */
    virtual void handleMessageAsNonHostPeer(std::unique_ptr<CommPeer> peer, const std::string &msgString) = 0;

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
    virtual void totalOrderBroadcast(const std::string &msg) = 0;

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
