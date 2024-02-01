//
// Created by simatic on 11/25/23.
//

#include <chrono>
#include <iostream>
#include "../../AlgoLayer/AlgoLayer.h"
#include "TcpCommLayer.h"
#include "TcpCommPeer.h"
#include "../../SessionLayer/SessionLayer.h"

using boost::asio::ip::tcp;
using namespace std;

/**
 * @brief Number of TCP connect tentatives before considering we cannot connect to desired host
 */
constexpr int nbTcpConnectTentatives{20};

/**
 * @brief Duration between two tentatives to connect to a host
 */
constexpr chrono::duration durationBetweenTcpConnectTentatives{500ms};

void TcpCommLayer::acceptConn(int port, size_t nbAwaitedConnections) {
    std::vector<std::future<void>> tasksHandleConn(nbAwaitedConnections);
    try
    {
        tcp::acceptor a(ioService, tcp::endpoint(tcp::v4(), static_cast<unsigned short>(port)));

        for (auto &t : tasksHandleConn)
        {
            auto ptrSock = make_unique<tcp::socket>(ioService);
            a.accept(*ptrSock);

            boost::asio::ip::tcp::no_delay option(true);
            ptrSock->set_option(option);

            t = std::async(std::launch::async, &TcpCommLayer::handleIncomingConn, this, std::move(ptrSock));
        }
    }
    catch (boost::system::system_error& e)
    {
        if (e.code() == boost::asio::error::address_in_use)
            cerr << "ERROR: Server cannot bind to port " << port << " (probably because there is an other server running and already bound to this port)\n";
        else
            cerr << "ERROR: Unexpected Boost Exception in task acceptConn: " << e.what() << "\n";
        exit(1);
    }
    for (auto &t : tasksHandleConn) {
        t.get();
    }
}

void TcpCommLayer::broadcastMsg(std::string && msg)
{
    std::shared_lock readerLock(rwMtxIncomingPeers);
    for (auto & ptrPeer : incomingPeers)
    {
        auto tmp{msg};
        ptrPeer->sendMsg(std::move(tmp));
    }
}
unique_ptr<tcp::socket>  TcpCommLayer::tryConnectToHost(HostTuple host)
{
    unique_ptr<tcp::socket> ptrSock{nullptr};
    try
    {
        // EndPoint creation
        tcp::resolver resolver(ioService);

        auto portAsString = to_string(get<PORT>(host));
        tcp::resolver::query query(tcp::v4(), get<HOSTNAME>(host), portAsString);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        ptrSock = make_unique<tcp::socket>(ioService);
        ptrSock->connect(*iterator);

        boost::asio::ip::tcp::no_delay option(true);
        ptrSock->set_option(option);
    }
    catch (boost::system::system_error& e)
    {
        if (e.code() == boost::asio::error::connection_refused)
            return nullptr;
        else
        {
            cerr << "ERROR: Unexpected Boost Exception in method tryConnectToHost: " << e.what() << "\n";
            exit(1);
        }
    }
    return ptrSock;
}

std::unique_ptr<CommPeer> TcpCommLayer::connectToHost(HostTuple host, AlgoLayer *aAlgoLayer)
{
    setAlgoLayer(aAlgoLayer);
    unique_ptr<tcp::socket> ptrSock{nullptr};
    for (int i = 0 ; i < nbTcpConnectTentatives ; ++i)
    {
        ptrSock = tryConnectToHost(host);
        if (ptrSock != nullptr)
        {
            auto peer = make_unique<TcpCommPeer>(ptrSock.get());
            tasksToJoin.emplace_back(std::async(std::launch::async, &TcpCommLayer::handleOutgoingConn, this, std::move(ptrSock)));
            return peer;
        }
        this_thread::sleep_for(durationBetweenTcpConnectTentatives);
    }
    cerr << "ERROR: Could not connect to server on machine \"" << get<HOSTNAME>(host) << "\", either because server is not started on this machine or it is not listening on port " << get<PORT>(host) << "\n";
    exit(1);
}

void TcpCommLayer::handleIncomingConn(std::unique_ptr<boost::asio::ip::tcp::socket> ptrSock) {
    TcpCommPeer tcpCommPeer{ptrSock.get()};
    {
        std::lock_guard writerLock(rwMtxIncomingPeers);
        incomingPeers.push_back(&tcpCommPeer);
    }
    try
    {
        for (;;)
        {
            auto s{receiveEvent(ptrSock.get())};
            std::scoped_lock lock(mtxCallbackHandleMessage);
            getAlgoLayer()->callbackHandleMessage(
                    make_unique<TcpCommPeer>(ptrSock.get()),
                    std::move(s));
        }
    }
    catch (boost::system::system_error& e)
    {
        if (e.code() == boost::asio::error::eof)
        {
            std::lock_guard writerLock(rwMtxIncomingPeers);
            erase_if(incomingPeers, [tcpCommPeer](const TcpCommPeer *t) { return t == &tcpCommPeer; });
            if (getAlgoLayer()->getSession()->getParam().getVerbose())
                cout << "Client disconnected\n";
        }
        else if (e.code() == boost::asio::error::connection_reset)
        {
            // Can only be experienced on Windows
            cerr << "ERROR: Client disconnected abnormally (probably because it crashed)\n";
            exit(1);
        }
        else
        {
            cerr << "ERROR: Unexpected Boost Exception in task handleIncomingConn: " << e.what() << "\n";
            exit(1);
        }
    }
}

void TcpCommLayer::handleOutgoingConn(std::unique_ptr<boost::asio::ip::tcp::socket> ptrSock) {
    try
    {
        for (;;)
        {
            auto s{receiveEvent(ptrSock.get())};
            std::scoped_lock lock(mtxCallbackHandleMessage);
            if (getAlgoLayer()->callbackHandleMessage(make_unique<TcpCommPeer>(ptrSock.get()), std::move(s)))
                break;
        }
    }
    catch (boost::system::system_error& e)
    {
        if (e.code() == boost::asio::error::eof || e.code() == boost::asio::error::connection_reset)
            // Server disconnection is signified with
            //  - boost::asio::error::eof for Linux
            //  - boost::asio::error::connection_reset for Windows
            cerr << "ERROR: Server has disconnected (probably because it crashed)\n";
        else
            cerr << "ERROR: Unexpected Boost Exception in task handleOutgoingConn: " << e.what() << "\n";
        exit(1);
    }
}

void TcpCommLayer::initHost(int port, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer)
{
    setAlgoLayer(aAlgoLayer);
    tasksToJoin.emplace_back(std::async(std::launch::async, &TcpCommLayer::acceptConn, this, port, nbAwaitedConnections));
}

std::string TcpCommLayer::receiveEvent(boost::asio::ip::tcp::socket *ptrSock)
{
    // We read the length of the message
    size_t len;
    boost::system::error_code error;
    auto length = boost::asio::read(*ptrSock, boost::asio::buffer(&len, sizeof(len)), error);
    if (error)
        throw boost::system::system_error(error); // Some other error. boost::asio::error::eof is the error which makes sense to look at
    assert(length == sizeof(len));
    // We read the message itself
    std::vector<char> v(len);
    auto msg_length = boost::asio::read(*ptrSock,
                                        boost::asio::buffer(v));
    assert(msg_length == len);
    return std::string{v.data(), v.size()};
}

std::string TcpCommLayer::toString() {
    return "TCP";
}

void TcpCommLayer::waitForMsg(size_t maxDisconnections) {
    // In TcpCommLayer, messages are received by handleIncomingConn and handleOutgoingConn tasks.
    // So we only have to wait for all these tasks to die (Note: handleOutgoing tasks are watched out by acceptConn
    // task ==> We only have to wait for acceptConn task to die).
    for (auto &t : tasksToJoin) {
        t.get();
    }
}
