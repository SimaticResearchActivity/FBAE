#pragma once

#include "memory"
#include "CommPeer.h"
#include "../Param.h"

class AlgoLayer;

class CommLayer {
private:
    AlgoLayer* algoLayer{nullptr};
public:
    virtual ~CommLayer() = default;

    /**
     * @brief Broadcast message contained in @msg to all peer connected to the" process
     * @param msg Message to be totalOrderBroadcast
     */
    virtual void broadcastMsg(std::string_view msg) = 0;

    /**
     * @brief Connect to host @host.
     * @param host Host to connect to.
     * @param algoLayer @AlgoLayer using this @CommLayer.
     * @return @CommPeer object which can be used to communicate with @host.
     */
    virtual std::unique_ptr<CommPeer> connectToHost(HostTuple host, AlgoLayer *algoLayer) = 0;

    /**
     * @brief Getter for @algoLayer.
     * @return @algoLayer.
     */
    [[nodiscard]] AlgoLayer* getAlgoLayer() const;

    /**
     * @brief Initializes accepting @nbAwaitedConnections connections on port @port. If such acceptance is blocking for
     * the considered communication protocol, blocks until @nbAwaitedConnections connections have been established.
     * @param port Socket port on which to wait for connections .
     * @param nbAwaitedConnections Number of disconnections to wait before returning (case of communication protocol
     * which is blocking on accepting connections).
     * @param aAlgoLayer @AlgoLayer using this @CommLayer.
     */
    virtual void initHost(int port, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) = 0;

    /**
     * @brief Setter for @algoLayer.
     * @param aAlgoLayer The value to give to @algoLayer.
     */
    void setAlgoLayer(AlgoLayer* aAlgoLayer);

    /**
     * @brief Return the name of the protocol used as @CommLayer
     * @return Name of the protocol used as @CommLayer
     */
    [[nodiscard]] virtual std::string toString() = 0;

    /**
     * @brief Waits to receive messages until there are @maxDisconnect disconnections. When a message is received,
     * calls @algoLayer->@handleMessageAsNonHostPeer() if @nonHostPeer is true and @algoLayer->@handleMessageAsHost()
     * otherwise.
     * @param nonHostPeer Must call @algoLayer->@handleMessageAsNonHostPeer() if @nonHostPeer is true and
     * @algoLayer->@handleMessageAsHost() otherwise.
     * @param maxDisconnections Maximum number of disconnections before returning.
     */
    virtual void waitForMsg(size_t maxDisconnections) = 0;
};
