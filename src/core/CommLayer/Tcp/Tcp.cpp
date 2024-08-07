//
// Created by simatic on 11/25/23.
//

#include "Tcp.h"

#include <cassert>
#include <chrono>

#include "../../AlgoLayer/AlgoLayer.h"
#include "../../SessionLayer/SessionLayer.h"
#include "cereal/archives/binary.hpp"

using boost::asio::as_tuple_t;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable_t;
using boost::asio::ip::tcp;
using default_token = as_tuple_t<use_awaitable_t<>>;
using tcp_acceptor = default_token::as_default_on_t<tcp::acceptor>;
namespace this_coro = boost::asio::this_coro;
using namespace std;
using namespace fbae::core;

namespace fbae::core::CommLayer::Tcp {

Tcp::Tcp() : CommLayer{"fbae.core.CommLayer.Tcp"} {}

/**
 * @brief Maximum number of TCP connect attempts before considering we cannot
 * connect to desired host
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
      boost::asio::ip::multicast::join_group(boost::asio::ip::make_address(
          arguments.getNetworkLevelMulticastAddress())));

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
    boost::asio::as_tuple_t<boost::asio::use_awaitable_t<>>::as_default_on_t<
        boost::asio::ip::tcp::socket>
        socket) {
  decrementNbAwaitedInitializationEvents();
  for (;;) {
    // Read message length
    size_t len;
    auto [e1, lenRead1] =
        co_await socket.async_read_some(boost::asio::buffer(&len, sizeof(len)));
    if (e1 == boost::asio::error::eof) {
      // Client disconnected
      break;
    }
    if (e1) {
      LOG4CXX_FATAL_FMT(getCommLogger(),
                        "Boost error '{}' not handled at: {}:{}", e1.what(),
                        path2file(std::source_location::current().file_name()),
                        std::source_location::current().line());
      exit(1);
    }
    assert(lenRead1 == sizeof(len));

    // Read message itself
    std::vector<char> v(len);
    auto buf{boost::asio::buffer(v, len)};
    auto lenRemainingToRead{len};
    do {
      auto [e2, lenRead2] =
        co_await socket.async_read_some(boost::asio::buffer(buf + (len - lenRemainingToRead), lenRemainingToRead));
      if (e2 == boost::asio::error::eof) {
        LOG4CXX_FATAL(getCommLogger(),
                      "Client disconnected between receiving message length and "
                      "receiving message");
        exit(1);
      }
      if (e2) {
        LOG4CXX_FATAL_FMT(getCommLogger(),
                          "Boost error '{}' not handled at: {}:{}", e2.what(),
                          path2file(std::source_location::current().file_name()),
                          std::source_location::current().line());
        exit(1);
      }
      lenRemainingToRead -= lenRead2;
    } while (lenRemainingToRead > 0);
    callbackReceive(std::string{v.data(), v.size()});
  }
}

awaitable<void> Tcp::coroutineServer(size_t nbAwaitedConnections) {
  auto executor = co_await this_coro::executor;
  tcp_acceptor acceptor(
      executor,
      tcp::endpoint(
          tcp::v4(),
          static_cast<unsigned short>(get<PORT>(
              getAlgoLayer()
                  ->getSessionLayer()
                  ->getArguments()
                  .getSites()[getAlgoLayer()->getSessionLayer()->getRank()]))));
  for (int i = 0; i < nbAwaitedConnections; ++i) {
    if (auto [e, socket] = co_await acceptor.async_accept(); socket.is_open()) {
      if (e) {
        LOG4CXX_FATAL_FMT(
            getCommLogger(), "Boost exception '{}' not handled at: {}:{}",
            e.what(), path2file(std::source_location::current().file_name()),
            std::source_location::current().line());
        exit(1);
      }
      boost::asio::ip::tcp::no_delay option(true);
      socket.set_option(option);
      co_spawn(executor, coroutineIncomingMessageReceiver(std::move(socket)),
               detached);
    }
  }
}

awaitable<void> Tcp::coroutineClient(rank_t rankClient) {
  auto executor = co_await this_coro::executor;
  auto ptrSocket = make_unique<tcp::socket>(executor);

  int nbTcpConnectAttempts{0};
  bool connected{false};
  do {
    auto [hostname, port]{getAlgoLayer()
                              ->getSessionLayer()
                              ->getArguments()
                              .getSites()[rankClient]};
    try {
      tcp::resolver resolver{executor};
      boost::asio::ip::basic_resolver_results<tcp> endpoints =
          co_await resolver.async_resolve(
              hostname, std::to_string(static_cast<unsigned short>(port)),
              boost::asio::use_awaitable);
      co_await boost::asio::async_connect(*ptrSocket, endpoints,
                                          boost::asio::use_awaitable);
      connected = true;
    } catch (boost::system::system_error& e) {
      if (e.code() == boost::asio::error::host_not_found) {
        LOG4CXX_FATAL_FMT(getCommLogger(), "Host '{}:{:d}' does not exist.",
                          hostname, port);
        exit(1);
      }
      if (e.code() != boost::asio::error::connection_refused) {
        LOG4CXX_FATAL_FMT(
            getCommLogger(), "Boost exception '{}' not handled at: {}:{}",
            e.what(), path2file(std::source_location::current().file_name()),
            std::source_location::current().line());
        exit(1);
      }
      ++nbTcpConnectAttempts;
      if (nbTcpConnectAttempts > maxNbTcpConnectAttempts) {
        LOG4CXX_FATAL_FMT(getCommLogger(),
                          "Connection to '{}:{:d}'  refused. Maybe no FBAE "
                          "instance is running on this hostname:port?",
                          hostname, port);
        exit(1);
      }
    }
    if (!connected) {
      boost::asio::steady_timer timer(executor);
      timer.expires_after(durationBetweenTcpConnectAttempts);
      co_await timer.async_wait(boost::asio::use_awaitable);
    }
  } while (!connected);
  boost::asio::ip::tcp::no_delay option(true);
  ptrSocket->set_option(option);
  addElementInRank2sock(rankClient, std::move(ptrSocket));
  decrementNbAwaitedInitializationEvents();
}

void Tcp::addElementInRank2sock(
    rank_t rank, std::unique_ptr<boost::asio::ip::tcp::socket> ptrSocket) {
  rank2sock[rank] = std::move(ptrSocket);
}

void Tcp::callbackReceive(string&& algoMsgAsString) {
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
    for (auto& msg : initDoneMessageBuffer) {
      getAlgoLayer()->callbackReceive(std::move(msg));
    }
    initDoneMessageBuffer.clear();
    initDone = true;
  }
}

void Tcp::multicastMsg(const std::string& algoMsgAsString) {
  if (getAlgoLayer()
          ->getSessionLayer()
          ->getArguments()
          .isUsingNetworkLevelMulticast()) {
    multicastSocket.send_to(boost::asio::buffer(algoMsgAsString),
                            multicastEndpoint);
  } else {
    for (auto const& [r, sock] : rank2sock) {
      send(r, algoMsgAsString);
    }
  }
}

void Tcp::openDestAndWaitIncomingMsg(std::vector<rank_t> const& dest,
                                     size_t nbAwaitedConnections,
                                     AlgoLayer::AlgoLayer* aAlgoLayer) {
  setAlgoLayer(aAlgoLayer);

  const auto arguments = getAlgoLayer()->getSessionLayer()->getArguments();
  const auto sites = arguments.getSites();
  const auto usingNetworkLevelMulticast =
      arguments.isUsingNetworkLevelMulticast();

  maxSizeForOneWrite = arguments.getIntInCommArgument("tcpMaxSizeForOneWrite", std::numeric_limits<int>::max());

  nbAwaitedInitializationEvents =
      nbAwaitedConnections  // nb hosts must connect to current host
      + dest.size()         // current host must connect to dest.size() hosts
      + (usingNetworkLevelMulticast
             ? 1
             : 0);  // Do we need to wait for MulticastReceiver to be ready

  try {
    if (usingNetworkLevelMulticast) {
      multicastEndpoint = {boost::asio::ip::make_address(
                               arguments.getNetworkLevelMulticastAddress()),
                           arguments.getNetworkLevelMulticastPort()};
      multicastSocket = {ioContext, multicastEndpoint.protocol()};
      co_spawn(ioContext, coroutineMulticastReceiver(), detached);
    }

    co_spawn(ioContext, coroutineServer(nbAwaitedConnections), detached);

    for (const auto rankClient : dest) {
      co_spawn(ioContext, coroutineClient(rankClient), detached);
    }

    ioContext.run();
  } catch (boost::system::system_error& e) {
    LOG4CXX_FATAL_FMT(getCommLogger(),
                      "Boost exception '{}' not handled at: {}:{}", e.what(),
                      path2file(std::source_location::current().file_name()),
                      std::source_location::current().line());
    exit(1);
  }
}

struct ForLength {
  size_t size{};

  // This method lets cereal know which data members to serialize
  template <class Archive>
  void serialize(Archive& archive) {
    archive(size);  // serialize things by passing them to the archive
  }
};

void Tcp::send(rank_t r, const std::string& algoMsgAsString) {
  assert(rank2sock.contains(r));
  ForLength forLength{algoMsgAsString.length()};
  std::stringstream oStream;
  {
    cereal::BinaryOutputArchive oarchive(oStream);  // Create an output archive
    oarchive(forLength);  // Write the data to the archive
  }  // archive goes out of scope, ensuring all contents are flushed
  auto sWithLength = oStream.str();

  if (algoMsgAsString.length() < maxSizeForOneWrite) {
    sWithLength.append(algoMsgAsString);
    boost::asio::write(*rank2sock[r], boost::asio::buffer(sWithLength));
  } else {
    boost::asio::write(*rank2sock[r], boost::asio::buffer(sWithLength));
    boost::asio::write(*rank2sock[r], boost::asio::buffer(algoMsgAsString));  
  }
}

void Tcp::terminate() {
  for (auto const& [r, sock] : rank2sock) {
    sock->close();
  }
  rank2sock.clear();
  if (getAlgoLayer()
          ->getSessionLayer()
          ->getArguments()
          .isUsingNetworkLevelMulticast() &&
      getAlgoLayer()->getSessionLayer()->getRank() == 0) {
    // Send an empty multicast to stop all receiveMulticast tasks
    multicastMsg("");
  }
}

std::string Tcp::toString() { return "TCP"; }

}  // namespace fbae::core::CommLayer::Tcp
