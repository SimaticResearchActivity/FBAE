#include <gtest/gtest.h>

#include "AlgoLayer/Trains/Trains.h"
#include "AlgoLayer/FMPI/FMPI.h"
#include "SessionLayer/SessionStub.h"
#include "msgTemplates.h"

namespace fbae::test::AlgoLayer {

using namespace std;
using namespace fbae::core;
using namespace fbae::core::AlgoLayer;
using namespace fbae::core::AlgoLayer::FMPI;
using namespace fbae::core::SessionLayer;

using enum fbae::core::SessionLayer::SessionMsgId;

using fbae::core::AlgoLayer::FMPI::FMPI;
using fbae::core::SessionLayer::SessionStub;

auto constexpr payloadA{"A"};
auto constexpr payloadB{"B"};
auto constexpr payloadC{"C"};
auto constexpr payloadD{"D"};

class FMPITest : public testing::Test {
 protected:
  void SetUp() override {
    algoLayer = make_unique<FMPI>();
    algoLayerRaw = algoLayer.get();
  }

  void commonChecks() {
    // Check @AlgoLayer:broadcastersGroup is correct
    ASSERT_EQ(nbSites, algoLayerRaw->getBroadcastersGroup().size());
    ASSERT_EQ(0, algoLayerRaw->getBroadcastersGroup()[0]);
    ASSERT_EQ(1, algoLayerRaw->getBroadcastersGroup()[1]);
    ASSERT_EQ(2, algoLayerRaw->getBroadcastersGroup()[2]);
    ASSERT_EQ(3, algoLayerRaw->getBroadcastersGroup()[3]);

    // Check Process is broadcasting messages
    ASSERT_TRUE(algoLayerRaw->isBroadcastingMessages());
  }

  int nbSites;
  int nbTrains;
  vector<HostTuple> sites;
  unique_ptr<FMPI> algoLayer;
  FMPI* algoLayerRaw;
  rank_t myRank;
};

TEST_F(FMPITest, ExecuteWith4SitesAndRank0) {
  nbSites = 4;
  sites = vector<HostTuple>(nbSites, HostTuple{"", 0});
  Arguments arguments{sites};
  myRank = 0;
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  nbTrains = 1;

  algoLayerRaw->execute();
  algoLayerRaw->terminate();

  commonChecks();

  // Check if TotalOrderBroadcast is empty
  ASSERT_EQ(0, algoLayerRaw->getBatchWaitingSessionMsg().size());

  // Check @SessionLayer::callbackInitDone() was called
  ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

  // Check no message was delivered
  ASSERT_EQ(0, sessionStub.getDelivered().size());
}

TEST_F(FMPITest, MPIMangerTest) {
  nbSites = 4;
  sites = vector<HostTuple>(nbSites, HostTuple{"", 0});
  Arguments arguments{sites};
  myRank = 0;
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  nbTrains = 1;

  algoLayerRaw->execute();
  algoLayerRaw->terminate();

  commonChecks();

  // Check if TotalOrderBroadcast is empty
  ASSERT_EQ(0, algoLayerRaw->getBatchWaitingSessionMsg().size());

  // Check @SessionLayer::callbackInitDone() was called
  ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

  // Check no message was delivered
  ASSERT_EQ(0, sessionStub.getDelivered().size());
}

}  // namespace fbae::test::AlgoLayer
