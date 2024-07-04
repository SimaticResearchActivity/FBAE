#pragma once

#include <latch>
#include <memory>
#include "../Arguments.h"
#include "../basicTypes.h"
#include "../Logger/LoggerConfig.h"


class AlgoLayer;

class CommLayer {
public:
    explicit CommLayer(std::string const& logger_name);
    virtual ~CommLayer() = default;

    /**
     * @brief Getter for @algoLayer.
     * @return @algoLayer.
     */
    [[nodiscard]] AlgoLayer* getAlgoLayer() const;

    /**
     * @brief Getter for @commLayerReady
     * @return @commLayerReady
     */
        [[nodiscard]] std::latch &getInitDoneCalled();

    /**
     * @brief Multicast @algoMsgAsString to all peers if Arguments::isUsingNetworkLevelMulticast is true
     * (network-level multicast) and only the peers which we connected to if Arguments::isUsingNetworkLevelMulticast is
     * false (software-level multicast).
     * @param algoMsgAsString Message to be totalOrderBroadcast
     */
    virtual void multicastMsg(const std::string &algoMsgAsString) = 0;

    /**
     * @brief Open connection to peers (named outgoing peers) which rank is listed in @dest, accepts
     * @nbAwaitedConnections connections from remote peers (named incoming peers), and waits for messages from
     * incoming peers until all these incoming peers have closed their connection with the current process.
     * @param dest Ranks of outgoing peer we must connect to.
     * @param nbAwaitedConnections Number of incoming peers which must connect to us.
     * @param aAlgoLayer @AlgoLayer using this @CommLayer.
     */
    virtual void openDestAndWaitIncomingMsg(std::vector<rank_t> const & dest, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) = 0;

    /**
     * @brief Setter for @algoLayer.
     * @param aAlgoLayer The value to give to @algoLayer.
     */
    void setAlgoLayer(AlgoLayer* aAlgoLayer);

    /**
     * @brief Sends @algoMsgAsString to outgoing peer of rank @r.
     * @param r Rank of outgoing peer
     * @param algoMsgAsString Message to send.
     */
    virtual void send(rank_t r, const std::string &algoMsgAsString) = 0;

    /**
     * @brief CommLayer must close all of its connections
     */
    virtual void terminate() = 0;

    /**
     * @brief Return the name of the protocol used as @CommLayer
     * @return Name of the protocol used as @CommLayer
     */
    [[nodiscard]] virtual std::string toString() = 0;

protected:
    /**
    * @brief Return the logger of the parent
    */
    [[nodiscard]] fbae::LoggerPtr getCommLogger() const;

private:
    AlgoLayer* algoLayer{nullptr};

    /**
     * @brief Latch used to guarantee that AlgoLayer::callbackInitDone() has been called before any call to
     * AlgoLayer::callbackReceive() is done by threads handling communication incoming messages.
     */
    std::latch initDoneCalled{1};

    /**
    * @brief Logger used to print information
    */
    fbae::LoggerPtr m_logger;
};