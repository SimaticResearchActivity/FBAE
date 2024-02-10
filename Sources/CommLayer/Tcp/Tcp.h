//
// Created by simatic on 11/25/23.
//

#pragma once

#include "boost/asio.hpp"
#include <future>
#include <map>
#include <mutex>
#include <shared_mutex>
#include "../CommLayer.h"
#include "../../Param.h"

class Tcp : public CommLayer{
public:
    Tcp() = default;
    void multicastMsg(std::string && msg) override;
    void openDestAndWaitIncomingMsg(std::vector<rank_t> const & dest, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) override;
    void send(rank_t r, std::string && msg) override;
    void terminate() override;
    std::string toString() override;

private:
    /**
     * @brief As Boost tutorial explains, all programs that use asio need to have at least one boost::asio::io_service
     * object. In fact, ioService must exist outside scope of the try catch in @acceptConn and @tryConnectToHost so that
     * its data can be referenced by ptrSock outside of these functions. We choose to define this object as an instance
     * variable.
     */
    boost::asio::io_service ioService;

    /**
     * @brief Mutex used to guarantee that all calls to callbackHandleMessage are done in a critical section.
     */
    std::mutex mtxCallbackHandleMessage;

    /**
     * @brief Mapping between rank of each outgoing peers and its associated socket
     */
     std::map<rank_t,std::unique_ptr<boost::asio::ip::tcp::socket>> rank2sock;

    /**
     * @brief Thread for accepting @nbAwaitedConnections connections on port @port
     * @param port
     * @param nbAwaitedConnections
     */
    void acceptConn(int port, size_t nbAwaitedConnections);

    /**
     * @brief Connect to host
    * @param host Host to connect to.
    * @return socket which can be used to communicate with @host.
    */
    std::unique_ptr<boost::asio::ip::tcp::socket> connectToHost(HostTuple const & host);

    /**
     * @brief Thread for handling messages received on @ptrSock (which was created by acceptConn)
     * @param ptrSock
     */
    void handleIncomingConn(std::unique_ptr<boost::asio::ip::tcp::socket> ptrSock);

    /**
     * @brief Waits till a packet is received on @psock
     * @param ptrSock
     * @return String containing received packet
     */
    static std::string receiveEvent(boost::asio::ip::tcp::socket *ptrSock);

    /**
     * @brief Tries to connect to host @host
     * @param host
     * @return unique_ptr containing socket if connection succeeds and nullptr if connection fails.
     */
    std::unique_ptr<boost::asio::ip::tcp::socket>  tryConnectToHost(HostTuple const & host);
};
