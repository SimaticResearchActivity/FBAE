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

    /**
     * @brief Add a new element in rank2sock
     * @param rank Rank of the element to add
     * @param socket Socket of the element to add
     */
    void addElementInRank2sock(rank_t rank, std::unique_ptr<boost::asio::ip::tcp::socket> ptrSocket);

    /**
     * @brief Decrements nbAwaitedInitializationEvents and calls @callbackInitDone() if it reaches 0
    */
    void decrementNbAwaitedInitializationEvents();

private:
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
