//
// Created by simatic on 11/25/23.
//

#pragma once
#include "CommPeer.h"
#include <boost/asio.hpp>


class TcpCommPeer : public CommPeer{
    boost::asio::ip::tcp::socket *psock;
public:
    explicit TcpCommPeer(boost::asio::ip::tcp::socket *psock);
    void disconnect() override;
    void sendMsg(std::string_view msg) override;
};

