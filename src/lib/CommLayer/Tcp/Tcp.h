//
// Created by simatic on 11/25/23.
//

#pragma once

#include <future>
#include <map>
#include <mutex>
#include <shared_mutex>
#include "boost/asio.hpp"
#include "../CommLayer.h"
#include "../../Arguments.h"

class Tcp : public CommLayer{
public:
    Tcp() = default;
    void multicastMsg(const std::string &algoMsgAsString) override;
    void openDestAndWaitIncomingMsg(std::vector<rank_t> const & dest, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) override;
    void send(rank_t r, const std::string &algoMsgAsString) override;
    void terminate() override;
    std::string toString() override;

private:
    /**
     * @brief Add a new element in rank2sock
     * @param rank Rank of the element to add
     * @param socket Socket of the element to add
     */
    void addElementInRank2sock(rank_t rank, std::unique_ptr<boost::asio::ip::tcp::socket> ptrSocket);

    /**
    * @brief Handles message received by one of the coroutines of @Tcp.cpp
    * @param algoMsgAsString String containing message.
    */
    void callbackReceive(std::string && algoMsgAsString);

    /**
     * @brief Coroutine to handle connection to another client.
     * @param rankClient Rank of the client to connect to.
     * @return
     */
    boost::asio::awaitable<void> coroutineClient(rank_t rankClient);

    /**
     * @brief Coroutine waiting asynchronously for unicasts received on an incoming connection and delivering them to @Tcp instance
     * @param socket Socket on which to wait for unicasts
     * @return
     */
    boost::asio::awaitable<void> coroutineIncomingMessageReceiver(boost::asio::as_tuple_t<boost::asio::use_awaitable_t<>>::as_default_on_t<boost::asio::ip::tcp::socket> socket);

    /**
     * @brief Coroutine taking care for waiting asynchronously for multicasts and delivering them to @Tcp instance.
     * @return
     */
    boost::asio::awaitable<void> coroutineMulticastReceiver();

    /**
     * @brief Coroutine to accept incoming connexions.
     * @param nbAwaitedConnections Number of connections to accept.
     * @return
     */
    boost::asio::awaitable<void> coroutineServer(size_t nbAwaitedConnections);

    /**
     * @brief Decrements nbAwaitedInitializationEvents and calls @callbackInitDone() if it reaches 0
    */
    void decrementNbAwaitedInitializationEvents();

    /**
     * @brief Flag used to determine if @callbackInitDone() was called.
     */
    bool initDone{false};

    /**
     * @brief Buffer for storing messages when @callbackReceive() is called and @initDone is false.
     */
    std::vector<std::string> initDoneMessageBuffer;

    /**
     * @brief ioContext used for asynchronous IO
     */
    boost::asio::io_context ioContext;

    /**
     * @brief Endpoint used to send network-level multicast
     */
    boost::asio::ip::udp::endpoint multicastEndpoint;

    /**
     * @brief Socket used to send network-level multicast
     */
    boost::asio::ip::udp::socket multicastSocket{ioContext};

    /**
     * @brief Number of initialization events to wait for before calling @callbackInitDone
     */
    size_t nbAwaitedInitializationEvents{0};

    /**
     * @brief Mapping between rank of each outgoing peers and its associated socket
     */
    std::map<rank_t,std::unique_ptr<boost::asio::ip::tcp::socket>> rank2sock;
};
