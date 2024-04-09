//
// Created by simatic on 11/25/23.
//

#include <cassert>
#include <chrono>
#include <iostream>
#include "../../AlgoLayer/AlgoLayer.h"
#include "../../SessionLayer/SessionLayer.h"
#include "Tcp.h"
#include "cereal/archives/binary.hpp"

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable_t;
using boost::asio::as_tuple_t;
using default_token = as_tuple_t<use_awaitable_t<>>;
using tcp_acceptor = default_token::as_default_on_t<tcp::acceptor>;
using tcp_socket = default_token::as_default_on_t<tcp::socket>;
namespace this_coro = boost::asio::this_coro;
using namespace std;

/**
 * @brief Maximum number of TCP connect attempts before considering we cannot connect to desired host
 */
constexpr int maxNbTcpConnectAttempts{20};

/**
 * @brief Duration between two tentatives to connect to a host
 */
constexpr chrono::duration durationBetweenTcpConnectAttempts{500ms};

/**
 * @brief Defines that any multicasting hosts is listened to.
 */
constexpr auto multicastListenToAnyone{"0.0.0.0"};

#ifdef WIN32
constexpr char dirSeparator = '\\';
#else
constexpr char dirSeparator = '/';
#endif

/**
 * @brief Given a filename including its path, returns only the filename
 * @param path including filename at the end.
 * @return filename without the path
 */
constexpr const char* path2file(const char* path) {
    const char* file = path;
    while (*path) {
        if (*path++ == dirSeparator) {
            file = path;
        }
    }
    return file;
}

awaitable<void> Tcp::coroutineMulticastReceiver() {
    auto executor = co_await this_coro::executor;
    boost::asio::ip::udp::socket socket{executor};

    auto arguments{getAlgoLayer()->getSessionLayer()->getArguments()};
    // Create the socket so that multiple may be bound to the same address.
    boost::asio::ip::udp::endpoint listen_endpoint(
            boost::asio::ip::make_address(multicastListenToAnyone),
            arguments.getNetworkLevelMulticastPort());
    socket.open(listen_endpoint.protocol());
    socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket.bind(listen_endpoint);

    // Join the multicast group.
    socket.set_option(
            boost::asio::ip::multicast::join_group(boost::asio::ip::make_address(arguments.getNetworkLevelMulticastAddress())));

    decrementNbAwaitedInitializationEvents();

    boost::asio::ip::udp::endpoint sender_endpoint;
    std::vector<char> v(maxLength);
    for (;;) {
        auto length = co_await socket.async_receive_from(
                boost::asio::buffer(v), sender_endpoint, boost::asio::use_awaitable);
        if (length == 0) {
            // Received empty message ==> Terminate
            break;
        }
        callbackReceive(std::string{v.data(), length});
    }
}

awaitable<void> Tcp::coroutineIncomingMessageReceiver(
        boost::asio::as_tuple_t<boost::asio::use_awaitable_t<>>::as_default_on_t<boost::asio::ip::tcp::socket> socket) {
    decrementNbAwaitedInitializationEvents();
    for (;;) {
        // Read message length
        size_t len;
        auto [e1, lenRead1] = co_await socket.async_read_some(boost::asio::buffer(&len, sizeof(len)));
        if (e1 == boost::asio::error::eof) {
            // Client disconnected
            break;
        }
        if (e1) {
            std::cerr << "Boost error '" << e1.what() << "' not handled at: "
                      << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
            exit(1);
        }
        assert(lenRead1 == sizeof(len));

        // Read message itself
        std::vector<char> v(len);
        auto [e2, lenRead2] = co_await socket.async_read_some(boost::asio::buffer(v));
        if (e2 == boost::asio::error::eof) {
            std::cerr << "ERROR : Client disconnected between receiving message length and receiving message\n";
            exit(1);
        }
        if (e2) {
            std::cerr << "Boost error '" << e2.what() << "' not handled at: "
                      << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
            exit(1);
        }
        assert(lenRead2 == len);
        callbackReceive(std::string{v.data(), v.size()});
    }
}

awaitable<void> Tcp::coroutineServer(size_t nbAwaitedConnections)
{
    auto executor = co_await this_coro::executor;
    tcp_acceptor acceptor(executor, tcp::endpoint(tcp::v4(),
                                                  static_cast<unsigned short>(get<PORT>(getAlgoLayer()->getSessionLayer()->getArguments().getSites()[
                                                          getAlgoLayer()->getSessionLayer()->getRank()]))));
    for (int i = 0 ; i < nbAwaitedConnections ; ++i) {
        if (auto [e, socket] = co_await acceptor.async_accept(); socket.is_open()) {
            if (e) {
                std::cerr << "Boost exception '" << e.what() << "' not handled at: "
                          << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                exit(1);

            }
            co_spawn(executor, coroutineIncomingMessageReceiver(std::move(socket)), detached);
        }
    }
}

awaitable<void> Tcp::coroutineClient(rank_t rankClient) {
    auto executor = co_await this_coro::executor;
    auto ptrSocket = make_unique<tcp::socket>(executor);

    int nbTcpConnectAttempts{0};
    bool connected{false};
    do {
        auto [hostname, port] {getAlgoLayer()->getSessionLayer()->getArguments().getSites()[rankClient]};
        try {
            tcp::resolver resolver{executor};
            boost::asio::ip::basic_resolver_results<tcp> endpoints =
                    co_await resolver.async_resolve(hostname, std::to_string(static_cast<unsigned short>(port)), boost::asio::use_awaitable);
            co_await boost::asio::async_connect(*ptrSocket, endpoints, boost::asio::use_awaitable);
            connected = true;
        }
        catch (boost::system::system_error& e)
        {
            if (e.code() == boost::asio::error::host_not_found) {
                std::cerr << "Error: Host '" << hostname << ":" << static_cast<unsigned short>(port) << "' does not exist.\n";
                exit(1);
            }
            if (e.code() != boost::asio::error::connection_refused) {
                std::cerr << "Boost exception '" << e.what() << "' not handled at: "
                          << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                exit(1);
            }
            ++nbTcpConnectAttempts;
            if (nbTcpConnectAttempts > maxNbTcpConnectAttempts) {
                std::cerr << "Error: Connection to '" << hostname << ":" << static_cast<unsigned short>(port) << "' refused. Maybe no FBAE instance is running on this hostname:port?\n";
                exit(1);
            }
        }
        if (!connected) {
            boost::asio::steady_timer timer(executor);
            timer.expires_after(durationBetweenTcpConnectAttempts);
            co_await timer.async_wait(boost::asio::use_awaitable);
        }
    } while (!connected);
    addElementInRank2sock(rankClient, std::move(ptrSocket));
    decrementNbAwaitedInitializationEvents();
}

void Tcp::addElementInRank2sock(rank_t rank, std::unique_ptr<boost::asio::ip::tcp::socket> ptrSocket) {
    rank2sock[rank] = std::move(ptrSocket);
}

void Tcp::callbackReceive(string &&algoMsgAsString) {
    if (initDone) {
        getAlgoLayer()->callbackReceive(std::move(algoMsgAsString));
    } else {
        initDoneMessageBuffer.emplace_back(std::move(algoMsgAsString));
    }
}

void Tcp::decrementNbAwaitedInitializationEvents() {
    --nbAwaitedInitializationEvents;
    if (nbAwaitedInitializationEvents == 0) {
        getAlgoLayer()->callbackInitDone();
        for (auto & msg: initDoneMessageBuffer) {
            getAlgoLayer()->callbackReceive(std::move(msg));
        }
        initDoneMessageBuffer.clear();
        initDone = true;
    }
}

void Tcp::multicastMsg(const std::string &algoMsgAsString) {
    if (getAlgoLayer()->getSessionLayer()->getArguments().isUsingNetworkLevelMulticast()) {
        multicastSocket.send_to(boost::asio::buffer(algoMsgAsString), multicastEndpoint);
    } else {
        for (auto const &[r, sock]: rank2sock) {
            send(r, algoMsgAsString);
        }
    }
}

void Tcp::openDestAndWaitIncomingMsg(std::vector<rank_t> const & dest, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) {
    setAlgoLayer(aAlgoLayer);
    const auto arguments = getAlgoLayer()->getSessionLayer()->getArguments();
    const auto sites = arguments.getSites();
    const auto usingNetworkLevelMulticast = arguments.isUsingNetworkLevelMulticast();

    nbAwaitedInitializationEvents =
            nbAwaitedConnections // nb hosts must connect to current host
            + dest.size()        // current host must connect to dest.size() hosts
            + (usingNetworkLevelMulticast ? 1 : 0); // Do we need to wait for MulticastReceiver to be ready

    try {
        if (usingNetworkLevelMulticast) {
            multicastEndpoint = {
                boost::asio::ip::make_address(arguments.getNetworkLevelMulticastAddress()),
                arguments.getNetworkLevelMulticastPort()};
            multicastSocket = {ioContext, multicastEndpoint.protocol()};
            co_spawn(ioContext, coroutineMulticastReceiver(), detached);
        }

        co_spawn(ioContext, coroutineServer(nbAwaitedConnections), detached);

        for (const auto rankClient: dest) {
            co_spawn(ioContext, coroutineClient(rankClient), detached);
        }

        ioContext.run();
    }
    catch (boost::system::system_error& e)
    {
        std::cerr << "Boost exception '" << e.what() << "' not handled at: "
                  << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
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
    if (getAlgoLayer()->getSessionLayer()->getArguments().isUsingNetworkLevelMulticast()
        && getAlgoLayer()->getSessionLayer()->getRank() == 0) {
        // Send an empty multicast to stop all receiveMulticast tasks
        multicastMsg("");
    }
}

std::string Tcp::toString() {
    return "TCP";
}