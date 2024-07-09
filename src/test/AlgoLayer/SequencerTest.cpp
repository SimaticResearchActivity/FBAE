//
// Created by simatic on 2/17/24.
//

#include <gtest/gtest.h>

#include "AlgoLayer/Sequencer/Sequencer.h"
#include "AlgoLayer/Sequencer/SequencerMsg.h"
#include "CommLayer/CommStub.h"
#include "SessionLayer/SessionStub.h"
#include "msgTemplates.h"

namespace fbae::test::AlgoLayer {

using namespace std;
using namespace fbae::core;
using namespace fbae::core::AlgoLayer;
using namespace fbae::core::AlgoLayer::Sequencer;
using namespace fbae::core::SessionLayer;

using enum fbae::core::SessionLayer::SessionMsgId;

using fbae::core::AlgoLayer::Sequencer::Sequencer;
using fbae::core::CommLayer::CommStub;
using fbae::core::SessionLayer::SessionStub;

constexpr auto nbSites = 4;

class SequencerTest : public testing::Test {
 protected:
  void SetUp() override {
    sites = vector<HostTuple>(nbSites, HostTuple{"", 0});
    commLayer = make_unique<CommStub>();
    commLayerRaw = commLayer.get();
    algoLayer = make_unique<Sequencer>(std::move(commLayer));
    algoLayerRaw = algoLayer.get();
  }

  vector<HostTuple> sites;
  unique_ptr<CommStub> commLayer;
  CommStub* commLayerRaw;
  unique_ptr<Sequencer> algoLayer;
  Sequencer* algoLayerRaw;
  rank_t myRank;
};

TEST_F(SequencerTest, SequencerExecute) {
  Arguments arguments{sites};
  myRank = sequencerRank;
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  // Check established connections
  ASSERT_EQ(nbSites - 1, commLayerRaw->getConnectedDest().size());
  EXPECT_EQ(1, commLayerRaw->getConnectedDest()[0]);
  EXPECT_EQ(2, commLayerRaw->getConnectedDest()[1]);
  EXPECT_EQ(3, commLayerRaw->getConnectedDest()[2]);

  // Check nbAwaitedConnections
  EXPECT_EQ(nbSites - 1, commLayerRaw->getNbAwaitedConnections());

  // Check no message was broadcast and thus sent.
  ASSERT_EQ(0, commLayerRaw->getSent().size());

  // Check @AlgoLayer:broadcastersGroup is correct
  ASSERT_EQ(nbSites - 1, algoLayerRaw->getBroadcastersGroup().size());
  EXPECT_EQ(1, algoLayerRaw->getBroadcastersGroup()[0]);
  EXPECT_EQ(2, algoLayerRaw->getBroadcastersGroup()[1]);
  EXPECT_EQ(3, algoLayerRaw->getBroadcastersGroup()[2]);

  // Check Sequencer is not broadcasting messages
  EXPECT_FALSE(algoLayerRaw->isBroadcastingMessages());

  // Check @SessionLayer::callbackInitDone() was called
  EXPECT_TRUE(sessionStub.isCallbackInitDoneCalled());

  // Check no message was delivered
  ASSERT_EQ(0, sessionStub.getDelivered().size());
}

TEST_F(SequencerTest, BroadcasterExecute) {
  Arguments arguments{sites};
  myRank = sequencerRank + 1;
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  // Check established connections
  ASSERT_EQ(1, commLayerRaw->getConnectedDest().size());
  EXPECT_EQ(sequencerRank, commLayerRaw->getConnectedDest()[0]);

  // Check nbAwaitedConnections
  EXPECT_EQ(1, commLayerRaw->getNbAwaitedConnections());

  // Check FirstBroadcast message was broadcast and thus sent.
  ASSERT_EQ(1, commLayerRaw->getSent().size());
  // Check that this message was sent to sequencer
  EXPECT_EQ(sequencerRank, commLayerRaw->getSent()[0].first);
  // Check contents of this message
  auto broadcastMsg{
      deserializeStruct<StructBroadcastMessage>(
          std::move(commLayerRaw->getSent()[0].second))};
  EXPECT_EQ(MsgId::BroadcastRequest,
            broadcastMsg.msgId);
  EXPECT_EQ(myRank - 1, broadcastMsg.senderPos);  // myRank-1 because senderPos
                                                  // is 1 less than senderRank
  EXPECT_EQ(FirstBroadcast,
            broadcastMsg.sessionMsg->msgId);

  // Check @AlgoLayer:broadcastersGroup is correct
  ASSERT_EQ(nbSites - 1, algoLayerRaw->getBroadcastersGroup().size());
  EXPECT_EQ(1, algoLayerRaw->getBroadcastersGroup()[0]);
  EXPECT_EQ(2, algoLayerRaw->getBroadcastersGroup()[1]);
  EXPECT_EQ(3, algoLayerRaw->getBroadcastersGroup()[2]);

  // Check plain Participant is broadcasting messages
  EXPECT_TRUE(algoLayerRaw->isBroadcastingMessages());

  // Check @SessionLayer::callbackInitDone() was called
  EXPECT_TRUE(sessionStub.isCallbackInitDoneCalled());

  // Check no message was delivered
  ASSERT_EQ(0, sessionStub.getDelivered().size());
}

TEST_F(SequencerTest, TotalOrderBroadcast) {
  Arguments arguments{sites};
  myRank = sequencerRank + 1;
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  commLayerRaw->getSent()
      .clear();  // We clear FirstBroadcast information which we already tested
                 // in previous execute() tests
  auto sessionMsg = make_shared<SessionTest>(TestMessage, "A");
  algoLayerRaw->totalOrderBroadcast(sessionMsg);

  // Check that 1 message was sent
  ASSERT_EQ(1, commLayerRaw->getSent().size());
  // Check that this message was sent to sequencer
  EXPECT_EQ(sequencerRank, commLayerRaw->getSent()[0].first);
  // Check contents of this message
  auto broadcastRequestMsg{
      deserializeStruct<StructBroadcastMessage>(
          std::move(commLayerRaw->getSent()[0].second))};
  EXPECT_EQ(MsgId::BroadcastRequest,
            broadcastRequestMsg.msgId);
  EXPECT_EQ(
      myRank - 1,
      broadcastRequestMsg
          .senderPos);  // myRank-1 because senderPos is 1 less than senderRank
  EXPECT_EQ(sessionMsg->msgId, broadcastRequestMsg.sessionMsg->msgId);
  EXPECT_EQ(sessionMsg->getPayload(),
            broadcastRequestMsg.sessionMsg->getPayload());

  // Check that no message is delivered
  ASSERT_EQ(0, sessionStub.getDelivered().size());
}

TEST_F(SequencerTest, SequencerReceivesBroadcastRequest) {
  Arguments arguments{sites};
  myRank = sequencerRank;
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  commLayerRaw->getSent()
      .clear();  // We clear FirstBroadcast information which we already tested
                 // in previous execute() tests
  auto sessionMsg = make_shared<SessionTest>(TestMessage, "A");
  constexpr rank_t senderPos = 42;
  auto algoMsgAsString{
      serializeStruct<StructBroadcastMessage>(
          StructBroadcastMessage{
              MsgId::BroadcastRequest, senderPos,
              sessionMsg})};
  algoLayerRaw->callbackReceive(std::move(algoMsgAsString));

  // Check that 3 messages are sent by Sequencer
  ASSERT_EQ(nbSites - 1, commLayerRaw->getSent().size());
  // Check that destinations of all of these messages
  EXPECT_EQ(1, commLayerRaw->getSent()[0].first);
  EXPECT_EQ(2, commLayerRaw->getSent()[1].first);
  EXPECT_EQ(3, commLayerRaw->getSent()[2].first);
  // Check second and third messages are equal to first message.
  EXPECT_EQ(commLayerRaw->getSent()[0].second,
            commLayerRaw->getSent()[1].second);
  EXPECT_EQ(commLayerRaw->getSent()[0].second,
            commLayerRaw->getSent()[2].second);
  // Check contents of the first message
  auto broadcastMsg{
      deserializeStruct<StructBroadcastMessage>(
          std::move(commLayerRaw->getSent()[0].second))};
  EXPECT_EQ(MsgId::Broadcast, broadcastMsg.msgId);
  EXPECT_EQ(senderPos, broadcastMsg.senderPos);
  EXPECT_EQ(sessionMsg->msgId, broadcastMsg.sessionMsg->msgId);
  EXPECT_EQ(sessionMsg->getPayload(), broadcastMsg.sessionMsg->getPayload());

  // Check that no message is delivered
  ASSERT_EQ(0, sessionStub.getDelivered().size());
}

TEST_F(SequencerTest, ParticipantReceivesBroadcast) {
  Arguments arguments{sites};
  myRank = sequencerRank + 1;
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  commLayerRaw->getSent()
      .clear();  // We clear FirstBroadcast information which we already tested
                 // in previous execute() tests
  auto sessionMsg = make_shared<SessionTest>(TestMessage, "A");
  constexpr rank_t senderPos = 42;
  auto algoMsgAsString{
      serializeStruct<StructBroadcastMessage>(
          StructBroadcastMessage{
              MsgId::Broadcast, senderPos,
              sessionMsg})};
  algoLayerRaw->callbackReceive(std::move(algoMsgAsString));

  // Check that no message has been sent
  ASSERT_EQ(0, commLayerRaw->getSent().size());

  // Check that 1 message has been delivered
  ASSERT_EQ(1, sessionStub.getDelivered().size());
  // Check sender of this message
  EXPECT_EQ(senderPos, sessionStub.getDelivered()[0].first);
  // Check contents of this message
  EXPECT_EQ(sessionMsg->msgId, sessionStub.getDelivered()[0].second->msgId);
  EXPECT_EQ(sessionMsg->getPayload(),
            sessionStub.getDelivered()[0].second->getPayload());
}

TEST_F(SequencerTest, ParticipantReceivesUnknownMessage) {
  Arguments arguments{sites};
  myRank = sequencerRank + 1;
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  commLayerRaw->getSent()
      .clear();  // We clear FirstBroadcast information which we already tested
                 // in previous execute() tests
  EXPECT_DEATH(
      {
        algoLayerRaw->callbackReceive(
            "z_thatIsMessageWithNonSenseMsgId which is 'z'");
      },
      ".*Unexpected msgId.*");  // Syntax of matcher is presented at
                                // https://google.github.io/googletest/advanced.html#regular-expression-syntax
}

}  // namespace fbae::test::AlgoLayer
