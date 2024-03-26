//
// Created by simatic on 11/25/23.
//

#include <cassert>
#include <chrono>
#include <iostream>
#include "../../AlgoLayer/AlgoLayer.h"
#include "Tcp.h"
#include "../../SessionLayer/SessionLayer.h"
#include "cereal/archives/binary.hpp"

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

void Tcp::acceptConn(int port, size_t nbAwaitedConnections) {
    std::vector<std::future<void>> tasksHandleConn(nbAwaitedConnections);
    try
    {
        tcp::acceptor a(ioServiceTcp, tcp::endpoint(tcp::v4(), static_cast<unsigned short>(port)));

        for (auto &t : tasksHandleConn)
        {
            auto ptrSock = make_unique<tcp::socket>(ioServiceTcp);
            a.accept(*ptrSock);

            boost::asio::ip::tcp::no_delay option(true);
            ptrSock->set_option(option);

            t = std::async(std::launch::async, &Tcp::handleIncomingConn, this, std::move(ptrSock));
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

std::unique_ptr<boost::asio::ip::tcp::socket>  Tcp::connectToHost(const HostTuple &host)
{
    unique_ptr<tcp::socket> ptrSock{nullptr};
    for (int i = 0 ; i < nbTcpConnectTentatives ; ++i)
    {
        ptrSock = tryConnectToHost(host);
        if (ptrSock != nullptr) {
            return ptrSock;
        }
        this_thread::sleep_for(durationBetweenTcpConnectTentatives);
    }
    cerr << "ERROR: Could not connect to server on machine \"" << get<HOSTNAME>(host) << "\", either because server is not started on this machine or it is not listening on port " << get<PORT>(host) << "\n";
    exit(1);
}

void Tcp::handleIncomingConn(std::unique_ptr<boost::asio::ip::tcp::socket> ptrSock) {
    try
    {
        getInitDoneCalled().wait();
        for (;;)
        {
            auto s{receiveEvent(ptrSock.get())};
            std::scoped_lock lock(mtxCallbackHandleMessage);
            getAlgoLayer()->callbackReceive(std::move(s));
        }
    }
    catch (boost::system::system_error& e)
    {
        if (e.code() == boost::asio::error::eof)
        {
            if (getAlgoLayer()->getSessionLayer()->getArguments().getVerbose())
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

void Tcp::multicastMsg(const std::string &algoMsgAsString) {
    if (getAlgoLayer()->getSessionLayer()->getArguments().getIsUsingNetworkLevelMulticast()) {
        multicastSocket.send_to(boost::asio::buffer(algoMsgAsString), multicastEndpoint);
    } else {
        for (auto const &[r, sock]: rank2sock) {
            send(r, algoMsgAsString);
        }
    }
}

void Tcp::openDestAndWaitIncomingMsg(std::vector<rank_t> const & dest, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) {
    setAlgoLayer(aAlgoLayer);
    const auto sites = getAlgoLayer()->getSessionLayer()->getArguments().getSites();
    auto rank = getAlgoLayer()->getSessionLayer()->getRank();

    future<void> taskReceiveMulticast;
    if (auto arguments = getAlgoLayer()->getSessionLayer()->getArguments() ; arguments.getIsUsingNetworkLevelMulticast()) {
        try {
            auto multicastAddress{boost::asio::ip::make_address(arguments.getNetworkLevelMulticastAddress())};
            multicastEndpoint = boost::asio::ip::udp::endpoint{multicastAddress, arguments.getNetworkLevelMulticastPort()};
            multicastSocket = boost::asio::ip::udp::socket{ioServiceUdp, multicastEndpoint.protocol()};
        }
        catch (boost::system::system_error& e)
        {
            cerr << "ERROR: Unexpected Boost Exception in method receiveMulticast: " << e.what()
                 << " (could be because multicast address, defined with -M program argument, is incorrect)\n";
            exit(1);
        }

        taskReceiveMulticast = std::async(&Tcp::receiveMulticast, this);
    }

    // Accept nbAwaitedConnections connections from incoming peers
    auto taskAcceptConn{ std::async(&Tcp::acceptConn, this, get < PORT > (sites[rank]), nbAwaitedConnections)};

    // Connect to outgoing peers listed in dest
    for (const auto &r: dest) {
        rank2sock[r] = connectToHost(sites[r]);
    }

    // We're done initializing @CommLayer
    getAlgoLayer()->callbackInitDone();
    getInitDoneCalled().count_down();

    taskAcceptConn.get();
    if (getAlgoLayer()->getSessionLayer()->getArguments().getIsUsingNetworkLevelMulticast()) {
        taskReceiveMulticast.get();
    }
}

std::string Tcp::receiveEvent(boost::asio::ip::tcp::socket *ptrSock)
{
    // Read the length of the message
    size_t len;
    boost::system::error_code error;
    auto length = boost::asio::read(*ptrSock, boost::asio::buffer(&len, sizeof(len)), error);
    if (error)
        throw boost::system::system_error(error); // Some other error. boost::asio::error::eof is the error which makes sense to look at
    assert(length == sizeof(len));
    // Read the message itself
    std::vector<char> v(len);
    auto msg_length = boost::asio::read(*ptrSock,
                                        boost::asio::buffer(v));
    assert(msg_length == len);
    return std::string{v.data(), v.size()};
}

void Tcp::receiveMulticast() {
    auto arguments = getAlgoLayer()->getSessionLayer()->getArguments();

    try {
        // Create the socket so that multiple may be bound to the same address.
        boost::asio::ip::address listenAddress{boost::asio::ip::make_address("0.0.0.0")}; // We listen to any hosts
        boost::asio::ip::udp::endpoint listenEndpoint(
                listenAddress, arguments.getNetworkLevelMulticastPort());

        boost::asio::ip::udp::socket sock{ioServiceUdp};
        sock.open(listenEndpoint.protocol());
        sock.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        sock.bind(listenEndpoint);

        // Join the multicast group.
        auto multicastAddress{boost::asio::ip::make_address(arguments.getNetworkLevelMulticastAddress())};
        sock.set_option(
                boost::asio::ip::multicast::join_group(multicastAddress));

        // Wait for multicasts until empty multicast
        size_t length;
        do {
            boost::asio::ip::udp::endpoint senderEndpoint;
            std::vector<char> v(maxLength);
            length = sock.receive_from(boost::asio::buffer(v), senderEndpoint);
            cout << "***MULTICAST*** Receive message of size " << length << "\n";
            if (length != 0) {
                std::string s{v.data(), length};
                std::scoped_lock lock(mtxCallbackHandleMessage);
                getAlgoLayer()->callbackReceive(std::move(s));
            }
        } while (length != 0);
    }
    catch (boost::system::system_error& e)
    {
       cerr << "ERROR: Unexpected Boost Exception in method receiveMulticast: " << e.what()
            << " (could be because multicast address, defined with -M program argument, is incorrect)\n";
       exit(1);
    }
}

struct ForLength
{
    size_t size{};

    // This method lets cereal know which data members to serialize
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(size); // serialize things by passing them to the archive
    }
};

void Tcp::send(rank_t r, const std::string &algoMsgAsString) {
    assert(rank2sock.contains(r));
    ForLength forLength{algoMsgAsString.length()};
    std::stringstream oStream;
    {
        cereal::BinaryOutputArchive oarchive(oStream); // Create an output archive
        oarchive(forLength); // Write the data to the archive
    } // archive goes out of scope, ensuring all contents are flushed

    auto sWithLength = oStream.str();
    sWithLength.append(algoMsgAsString);

    boost::asio::write(*rank2sock[r], boost::asio::buffer(sWithLength));
}

void Tcp::terminate() {
    for (auto const& [r, sock]: rank2sock) {
        sock->close();
    }
    rank2sock.clear();
    if (getAlgoLayer()->getSessionLayer()->getArguments().getIsUsingNetworkLevelMulticast()) {
        if (getAlgoLayer()->getSessionLayer()->getRank() == 0) {
            // Send an empty multicast to stop all receiveMulticast tasks
            cout << "***MULTICAST*** Send message of size 0\n";
            multicastMsg("");
        }
    }
}

unique_ptr<tcp::socket>  Tcp::tryConnectToHost(const HostTuple &host)
{
    unique_ptr<tcp::socket> ptrSock{nullptr};
    try
    {
        // EndPoint creation
        tcp::resolver resolver(ioServiceTcp);

        auto portAsString = to_string(get<PORT>(host));
        tcp::resolver::query query(tcp::v4(), get<HOSTNAME>(host), portAsString);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        ptrSock = make_unique<tcp::socket>(ioServiceTcp);
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

std::string Tcp::toString() {
    return "TCP";
}
