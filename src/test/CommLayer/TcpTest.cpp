//
// Created by simatic on 05/04/202424.
//

#include <gtest/gtest.h>

#include <future>

#include "AlgoLayer/AlgoStub.h"
#include "CommLayer/Tcp/Tcp.h"
#include "SessionLayer/SessionStub.h"

namespace fbae::test::CommLayer {

using namespace std;
using namespace fbae::core;

using fbae::core::AlgoLayer::AlgoStub;
using fbae::core::CommLayer::CommLayer;
using fbae::core::CommLayer::Tcp::Tcp;
using fbae::core::SessionLayer::SessionStub;
using fbae::core::AlgoLayer::InitDoneSupervisor;

/**
 * @brief Delay to wait between message sending and receive variable content
 * testing in order to be sure message was received and processed.
 */
constexpr chrono::duration delayBeforeTestingReceived{50ms};

/**
 * @brief Initial port used by TcpTest
 */
constexpr int initialPort = 11000;

class Host {
 public:
  Host(rank_t rank, std::vector<rank_t> const &dest,
       size_t nbAwaitedConnections, const Arguments &arguments,
       InitDoneSupervisor &initDoneSupervisor)
      : algo{make_unique<AlgoStub>(std::move(comm), initDoneSupervisor)},
        algoRaw{algo.get()},
        sessionStub{arguments, rank, std::move(algo)},
        async{std::async(std::launch::async,
                         [dest, nbAwaitedConnections, this]() {
                           commRaw->openDestAndWaitIncomingMsg(
                               dest, nbAwaitedConnections, algoRaw);
                         })} {}

  AlgoStub *getAlgoRaw() { return algoRaw; }

  CommLayer *getCommRaw() { return commRaw; }

  void waitAsync() { async.get(); }

 private:
  unique_ptr<Tcp> comm{make_unique<Tcp>()};
  CommLayer *commRaw{comm.get()};
  unique_ptr<AlgoStub> algo;
  AlgoStub *algoRaw;
  SessionStub sessionStub;
  future<void> async;
};

TEST(TcpTest, Main) {
  GTEST_SKIP() << "(Test skipped because its execution takes time).";
  // Test scenario is presented in doc/mscCommLayerTest.pu

  InitDoneSupervisor initDoneSupervisor;
  const std::vector<HostTuple> sites{{"localhost", initialPort},
                                     {"localhost", initialPort + 1},
                                     {"localhost", initialPort + 2},
                                     {"localhost", initialPort + 3}};
  Arguments arguments023{sites, "", "", true};
  Arguments arguments1{sites, "", "", false};

  // Using a vector of Host does not compile ==> We use host0, host1, etc.
  // instead of host[i].
  Host host0{0, {1}, 1, arguments023, initDoneSupervisor};
  Host host1{1, {0, 2}, 1, arguments1, initDoneSupervisor};

  initDoneSupervisor.waitInitDone();

  constexpr auto msgA{"A"};
  host0.getCommRaw()->send(1, msgA);
  std::this_thread::sleep_for(delayBeforeTestingReceived);
  ASSERT_EQ(0, host1.getAlgoRaw()->getReceived().size());

  Host host2{2, {3}, 1, arguments023, initDoneSupervisor};

  initDoneSupervisor.waitInitDone();

  ASSERT_EQ(1, host1.getAlgoRaw()->getReceived().size());
  EXPECT_EQ(msgA, host1.getAlgoRaw()->getReceived()[0]);
  host1.getAlgoRaw()->getReceived().clear();

  constexpr auto msgB{"B"};
  host1.getCommRaw()->send(2, msgB);
  std::this_thread::sleep_for(delayBeforeTestingReceived);
  ASSERT_EQ(0, host2.getAlgoRaw()->getReceived().size());

  Host host3{3, {}, 1, arguments023, initDoneSupervisor};

  initDoneSupervisor.waitInitDone();
  initDoneSupervisor.waitInitDone();

  ASSERT_EQ(1, host2.getAlgoRaw()->getReceived().size());
  EXPECT_EQ(msgB, host2.getAlgoRaw()->getReceived()[0]);
  host2.getAlgoRaw()->getReceived().clear();

  // Check multicast
  constexpr auto msgC{"C"};
  host1.getCommRaw()->multicastMsg(msgC);
  std::this_thread::sleep_for(delayBeforeTestingReceived);
  ASSERT_EQ(1, host0.getAlgoRaw()->getReceived().size());
  EXPECT_EQ(msgC, host0.getAlgoRaw()->getReceived()[0]);
  host0.getAlgoRaw()->getReceived().clear();
  ASSERT_EQ(1, host2.getAlgoRaw()->getReceived().size());
  EXPECT_EQ(msgC, host2.getAlgoRaw()->getReceived()[0]);
  host2.getAlgoRaw()->getReceived().clear();
  ASSERT_EQ(0, host1.getAlgoRaw()->getReceived().size());
  ASSERT_EQ(0, host3.getAlgoRaw()->getReceived().size());

  constexpr auto msgD{"D"};
  host0.getCommRaw()->multicastMsg(msgD);
  std::this_thread::sleep_for(delayBeforeTestingReceived);
  ASSERT_EQ(1, host0.getAlgoRaw()->getReceived().size());
  EXPECT_EQ(msgD, host0.getAlgoRaw()->getReceived()[0]);
  host0.getAlgoRaw()->getReceived().clear();
  ASSERT_EQ(1, host2.getAlgoRaw()->getReceived().size());
  EXPECT_EQ(msgD, host2.getAlgoRaw()->getReceived()[0]);
  host2.getAlgoRaw()->getReceived().clear();
  ASSERT_EQ(1, host3.getAlgoRaw()->getReceived().size());
  EXPECT_EQ(msgD, host3.getAlgoRaw()->getReceived()[0]);
  host3.getAlgoRaw()->getReceived().clear();
  ASSERT_EQ(0, host1.getAlgoRaw()->getReceived().size());

  // Check termination
  host0.getCommRaw()->terminate();
  host1.getCommRaw()->terminate();
  host2.getCommRaw()->terminate();
  host3.getCommRaw()->terminate();

  host0.waitAsync();
  host1.waitAsync();
  host2.waitAsync();
  host3.waitAsync();
}

}  // namespace fbae::test::CommLayer