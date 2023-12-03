//
// Created by simatic on 11/25/23.
//

#pragma once

#include <boost/asio.hpp>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include "CommLayer.h"
#include "Param.h"
#include "TcpCommPeer.h"

class TcpCommLayer : public CommLayer{
private:
    /**
     * @brief Peers created by threadAcceptConn
     */
    std::vector<TcpCommPeer*> incomingPeers;
    /**
     * @brief Mutex used to guarantee that all calls to callbackHandleMessage are done in a critical section.
     */
    std::mutex mtxCallbackHandleMessage;
    /**
     * @brief Mutex for controlling access to @incomingPeers
     */
    std::shared_timed_mutex rwMtxIncomingPeers;
    /**
     * @brief Threads created by TcpCommLayer which must be joined before existing an instance of TcpCommLayer
     */
    std::vector<std::thread> threadsToJoin;

    /**
     * @brief Thread for accepting @nbAwaitedConnections connections on port @port
     * @param port
     * @param nbAwaitedConnections
     */
    void acceptConn(int port, size_t nbAwaitedConnections);
    /**
     * @brief Thread for handling messages received on @ptrSock (which was created by acceptConn)
     * @param ptrSock
     */
    void handleIncomingConn(std::unique_ptr<boost::asio::ip::tcp::socket> ptrSock);
    /**
     * @brief Thread for handling messages received on @ptrSock (which was created by connectToHost)
     * @param ptrSock
     */
    void handleOutgoingConn(std::unique_ptr<boost::asio::ip::tcp::socket> ptrSock);
    /**
     * @brief Waits till a packet is received on @psock
     * @param ptrSock
     * @return String containing received packet
     */
    static std::string receiveEvent(boost::asio::ip::tcp::socket *ptrSock);
public:
    TcpCommLayer() = default;
    void broadcastMsg(std::string_view msg) override;
    std::unique_ptr<CommPeer> connectToHost(HostTuple host, AlgoLayer *algoLayer) override;
    void initHost(int port, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) override;
    std::string toString() override;
    void waitForMsg(size_t maxDisconnections) override;

};
