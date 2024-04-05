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
using namespace std;

/**
 * @brief Maximum number of TCP connect tentatives before considering we cannot connect to desired host
 */
constexpr int maxNbTcpConnectTentatives{20};

/**
 * @brief Duration between two tentatives to connect to a host
 */
constexpr chrono::duration durationBetweenTcpConnectTentatives{500ms};

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

/**
 * @brief Private class taking care for waiting asynchronously for multicasts and delivering them to @AlgoLayer.
 */
class MulticastReceiver : public std::enable_shared_from_this<MulticastReceiver> {
public:
    MulticastReceiver(boost::asio::io_context& ioContext,
                      Tcp *tcpCommLayer)
            : socket{ioContext}
            , tcp{tcpCommLayer}
    {
    }
    void start() {
        auto arguments{tcp->getAlgoLayer()->getSessionLayer()->getArguments()};
        // Create the socket so that multiple may be bound to the same address.
        boost::asio::ip::udp::endpoint listen_endpoint(
                boost::asio::ip::make_address(multicastListenToAnyone),
                arguments.getNetworkLevelMulticastPort());
        socket.open(listen_endpoint.protocol());
        socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket.bind(listen_endpoint);
        // Join the multicast group.
        socket.set_option(
                boost::asio::ip::multicast::join_group(
                        boost::asio::ip::make_address(arguments.getNetworkLevelMulticastAddress())));

        tcp->decrementNbAwaitedInitializationEvents();
        doReceiveMulticast();
    }

private:
    void doReceiveMulticast()
    {
        auto self(shared_from_this());
        std::vector<char> v(maxLength);
        boost::asio::ip::udp::endpoint sender_endpoint;
        socket.async_receive_from(
                boost::asio::buffer(v), sender_endpoint,
                [this, self, &v](boost::system::error_code ec, std::size_t length)
                {
                    if (ec) {
                        std::cerr << "Boost error '" << ec.what() << "' not handled at: "
                                  << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                        exit(1);
                    }
                    assert(length == v.size());
                    if (length != 0) {
                        tcp->getAlgoLayer()->callbackReceive(std::string{v.data(), v.size()});
                        doReceiveMulticast();
                    }else{
                        std::cout << "Received empty message ==> Terminate\n";
                    }
                });
    }

    boost::asio::ip::udp::socket socket;
    Tcp *tcp;
};

/**
 * @brief Private class taking care for waiting asynchronously for unicasts received on an incoming connection
 * and delivering them to @AlgoLayer.
 */
class TcpIncomingSession : public std::enable_shared_from_this<TcpIncomingSession> {
public:
    TcpIncomingSession(tcp::socket socket, Tcp *tcpCommLayer)
            : socket{std::move(socket)}
            , tcpCommLayer{tcpCommLayer}
    {
    }

    void start() {
        doReadLength();
    }

private:
    void doReadLength() {
        auto self(shared_from_this());
        size_t len;
        socket.async_read_some(boost::asio::buffer(&len, sizeof(len)),
                                [this, self, &len](boost::system::error_code ec, std::size_t length)
                                {
                                    if (ec == boost::asio::error::eof) {
                                        std::cout << "Client disconnected\n";
                                        return;
                                    }
                                    if (ec) {
                                        std::cerr << "Boost error '" << ec.what() << "' not handled at: "
                                                  << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                                        exit(1);
                                    }
                                    assert(length == sizeof(len));
                                    doReadMsg(len);
                                });
    }

    void doReadMsg(size_t len) {
        auto self(shared_from_this());
        std::vector<char> v(len);
        socket.async_read_some(boost::asio::buffer(v),
                               [this, len, &v, self](boost::system::error_code ec, std::size_t length)
                               {
                                   if (ec == boost::asio::error::eof) {
                                       std::cout << "Client disconnected (between receiving length and receiving message ==> Bizarre\n";
                                       return;
                                   }
                                   if (ec) {
                                       std::cerr << "Boost error '" << ec.what() << "' not handled at: "
                                                 << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                                       exit(1);
                                   }
                                   assert(length == len);
                                   tcpCommLayer->getAlgoLayer()->callbackReceive(std::string{v.data(), v.size()});
                                   doReadLength();
                               });
    }

    tcp::socket socket;
    Tcp *tcpCommLayer;
};

/**
 * @brief Private class taking care for waiting asynchronously connections to current host.
 */
class TcpServer {
public:
    TcpServer(boost::asio::io_context& ioContext, size_t nbAwaitedConnections, Tcp *tcpCommLayer)
            : acceptor(ioContext, tcp::endpoint(tcp::v4(),
                                                static_cast<unsigned short>(get<PORT>(tcpCommLayer->getAlgoLayer()->getSessionLayer()->getArguments().getSites()[
                                                        tcpCommLayer->getAlgoLayer()->getSessionLayer()->getRank()]))))
            , nbAwaitedConnections{nbAwaitedConnections}
            , socket{ioContext}
            , tcpCommLayer{tcpCommLayer}
    {
            do_accept();
    }

private:
    void do_accept() {
        if (nbAwaitedConnections > 0) {
            acceptor.async_accept(socket,
                                  [this](boost::system::error_code ec) {
                                      if (ec) {
                                          std::cerr << "Boost error '" << ec.what() << "' not handled at: "
                                                    << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                                          exit(1);
                                      }
                                      std::make_shared<TcpIncomingSession>(std::move(socket), tcpCommLayer)->start();
                                      --nbAwaitedConnections;
                                      tcpCommLayer->decrementNbAwaitedInitializationEvents();
                                      do_accept();
                                  });
        } else {
            std::cout << "Stop accepting TCP connections \n";
        }
    }

    tcp::acceptor acceptor;
    size_t nbAwaitedConnections;
    tcp::socket socket;
    Tcp *tcpCommLayer;
};

/**
 * @brief Private class taking care for connecting to a distant host.
 */
class TcpClient : public std::enable_shared_from_this<TcpClient> {
public:
    TcpClient(boost::asio::io_context& ioContext, rank_t rankClient, Tcp *tcpCommLayer)
            : ptrSocket{make_unique<tcp::socket>(ioContext)}
            , rankClient{rankClient}
            , resolver{ioContext}
            , tcpCommLayer{tcpCommLayer}
            , timer(ioContext)
    {
    }

    void start()
    {
        doAsyncResolve();
    }

private:
    void doAsyncConnect(tcp::resolver::iterator it)
    {
        auto self(shared_from_this());
        ptrSocket->async_connect(*it, [this, self, &it](boost::system::error_code ec)
        {
            if (ec == boost::asio::error::connection_refused) {
                --remainingNbTcpConnectTentatives;
                if (remainingNbTcpConnectTentatives == 0) {
                    auto [hostname, port] {tcpCommLayer->getAlgoLayer()->getSessionLayer()->getArguments().getSites()[rankClient]};
                    std::cerr << "Error: Connection to '" << hostname << ":" << static_cast<unsigned short>(port) << "' refused. Maybe server is not running on that port?\n";
                    exit(1);
                }
                doTimeoutBeforeNewAsyncConnect(std::move(it));
            }
            if (ec) {
                std::cerr << "Boost error '" << ec.what() << "' not handled at: "
                    << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                exit(1);
            }
            tcpCommLayer->addElementInRank2sock(rankClient, std::move(ptrSocket));
            tcpCommLayer->decrementNbAwaitedInitializationEvents();
        });
    }

    void doAsyncResolve()
    {
        auto self(shared_from_this());
        auto [hostname, port] {tcpCommLayer->getAlgoLayer()->getSessionLayer()->getArguments().getSites()[rankClient]};
        tcp::resolver::query q{hostname, std::to_string(static_cast<unsigned short>(port))};
        resolver.async_resolve(q, [this, self, hostname, port](boost::system::error_code ec, tcp::resolver::iterator it)
        {
            if (ec == boost::asio::error::host_not_found) {
                std::cerr << "Error: Host '" << hostname << ":" << static_cast<unsigned short>(port) << "' does not exist.\n";
                exit(1);
            }
            if (ec) {
                std::cerr << "Boost error '" << ec.what() << "' not handled at: "
                          << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                exit(1);
            }
            doAsyncConnect(std::move(it));
        });
    }

    void doTimeoutBeforeNewAsyncConnect(tcp::resolver::iterator it)
    {
        auto self(shared_from_this());
        timer.expires_after(std::chrono::milliseconds(durationBetweenTcpConnectTentatives));
        timer.async_wait(
                [this, self, &it](boost::system::error_code ec)
                {
                    if (ec) {
                        std::cerr << "Boost error '" << ec.what() << "' not handled at: "
                                  << path2file(std::source_location::current().file_name()) << ':' << std::source_location::current().line() << "\n";
                        exit(1);
                    }
                    doAsyncConnect(std::move(it));
                });

    }

    unique_ptr<tcp::socket> ptrSocket{nullptr};
    rank_t rankClient;
    tcp::resolver resolver;
    Tcp *tcpCommLayer;
    boost::asio::steady_timer timer;
    int remainingNbTcpConnectTentatives{maxNbTcpConnectTentatives};
};

void Tcp::addElementInRank2sock(rank_t rank, std::unique_ptr<boost::asio::ip::tcp::socket> ptrSocket) {
    rank2sock[rank] = std::move(ptrSocket);
}

void Tcp::decrementNbAwaitedInitializationEvents() {
    --nbAwaitedInitializationEvents;
    if (nbAwaitedInitializationEvents == 0) {
        getAlgoLayer()->callbackInitDone();
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
            std::make_shared<MulticastReceiver>(ioContext, this)->start();
        }

        TcpServer s(ioContext, nbAwaitedConnections, this);

        for (const auto rankClient: dest) {
            std::make_shared<TcpClient>(ioContext, rankClient, this)->start();
        }

        ioContext.run();
        std::cout << "*** Exit io_context.run(); ***\n";
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
        cout << "***MULTICAST*** Send message of size 0\n";
        multicastMsg("");
    }
}

std::string Tcp::toString() {
    return "TCP";
}