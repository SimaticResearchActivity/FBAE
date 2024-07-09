//
// Created by simatic on 2/17/24.
//

#include <gtest/gtest.h>

#include "../CommLayer/CommStub.h"
#include "../SessionLayer/SessionStub.h"
#include "AlgoLayer/LCR/LCR.h"
#include "msgTemplates.h"

namespace fbae::test::AlgoLayer {

using namespace std;
using namespace fbae::core;
using namespace fbae::core::AlgoLayer;
using namespace fbae::core::AlgoLayer::LCR;
using namespace fbae::core::SessionLayer;

using enum fbae::core::SessionLayer::SessionMsgId;

using fbae::core::AlgoLayer::LCR::LCR;
using fbae::core::CommLayer::CommStub;
using fbae::core::SessionLayer::SessionStub;

constexpr auto nbSites = 4;

class LCRTest : public testing::Test {
 protected:
  void SetUp() override {
    sites = vector<HostTuple>(nbSites, HostTuple{"", 0});
    commLayer = make_unique<CommStub>();
    commLayerRaw = commLayer.get();
    algoLayer = make_unique<LCR>(std::move(commLayer));
    algoLayerRaw = algoLayer.get();
    myRank = 3;
  }

  vector<HostTuple> sites;
  unique_ptr<CommStub> commLayer;
  CommStub* commLayerRaw;
  unique_ptr<LCR> algoLayer;
  LCR* algoLayerRaw;
  rank_t myRank;
};

// This test tests all the setup of the LCR layer.
TEST_F(LCRTest, LCRExecute) {
  Arguments arguments{sites};
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  // Check established connection
  ASSERT_EQ(1, commLayerRaw->getConnectedDest().size());
  ASSERT_EQ(0, commLayerRaw->getConnectedDest()[0]);

  // Check nbAwaitedConnections
  ASSERT_EQ(1, commLayerRaw->getNbAwaitedConnections());

  // Check @AlgoLayer:broadcastersGroup is correct
  ASSERT_EQ(nbSites, algoLayerRaw->getBroadcastersGroup().size());
  ASSERT_EQ(0, algoLayerRaw->getBroadcastersGroup()[0]);
  ASSERT_EQ(1, algoLayerRaw->getBroadcastersGroup()[1]);
  ASSERT_EQ(2, algoLayerRaw->getBroadcastersGroup()[2]);
  ASSERT_EQ(3, algoLayerRaw->getBroadcastersGroup()[3]);

  // Check Process is broadcasting messages
  ASSERT_TRUE(algoLayerRaw->isBroadcastingMessages());

  // Check FirstBroadcast message was broadcast and thus sent.
  ASSERT_EQ(1, commLayerRaw->getSent().size());
  // Check that this message was sent to process 0
  ASSERT_EQ(0, commLayerRaw->getSent()[0].first);
  // Check contents of this message
  auto broadcastMsg{deserializeStruct<MessagePacket>(
      std::move(commLayerRaw->getSent()[0].second))};
  ASSERT_EQ(MessageId::Message, broadcastMsg.messageId);
  ASSERT_EQ(myRank, broadcastMsg.senderRank);
  ASSERT_EQ(1, broadcastMsg.clock);
  ASSERT_EQ(FirstBroadcast, broadcastMsg.sessionMessage->msgId);

  // Check @SessionLayer::callbackInitDone() was called
  ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

  // Check no message was delivered
  ASSERT_EQ(0, sessionStub.getDelivered().size());
}

// This tests the behavior of the LCR when a site receives a message and is
// expected to return a message.
TEST_F(LCRTest, MessageToMessage) {
  Arguments arguments{sites};
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  // The original message that was sent (tested before).
  auto firstIncomingMessage{deserializeStruct<MessagePacket>(
      std::move(commLayerRaw->getSent()[0].second))};

  auto message =
      MessagePacket{.messageId = MessageId::Message,
                    .senderRank = 2,
                    .clock = 1,
                    .sessionMessage = firstIncomingMessage.sessionMessage};

  ASSERT_EQ(1, commLayerRaw->getSent().size());

  // Send the message to the current site.
  algoLayerRaw->callbackReceive(serializeStruct(message));

  ASSERT_EQ(2, commLayerRaw->getSent().size());

  auto secondIncomingMessage{deserializeStruct<MessagePacket>(
      std::move(commLayerRaw->getSent()[1].second))};
  ASSERT_EQ(MessageId::Message, secondIncomingMessage.messageId);
  ASSERT_EQ(2, secondIncomingMessage.senderRank);
  ASSERT_EQ(1, secondIncomingMessage.clock);
  ASSERT_EQ(FirstBroadcast,
            secondIncomingMessage.sessionMessage->msgId);
}

// This tests the behavior of the LCR when a site receives a message and is
// expected to return an acknowledgement.
TEST_F(LCRTest, MessageToAcknowledgement) {
  Arguments arguments{sites};
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  // The original message that was sent (tested before).
  auto firstIncomingMessage{deserializeStruct<MessagePacket>(
      std::move(commLayerRaw->getSent()[0].second))};

  auto message =
      MessagePacket{.messageId = MessageId::Message,
                    .senderRank = 0,
                    .clock = 1,
                    .sessionMessage = firstIncomingMessage.sessionMessage};

  ASSERT_EQ(1, commLayerRaw->getSent().size());

  // Send the message to the current site.
  algoLayerRaw->callbackReceive(serializeStruct(message));

  ASSERT_EQ(2, commLayerRaw->getSent().size());

  auto secondIncomingMessage{deserializeStruct<MessagePacket>(
      std::move(commLayerRaw->getSent()[1].second))};
  ASSERT_EQ(MessageId::Acknowledgement, secondIncomingMessage.messageId);
  ASSERT_EQ(0, secondIncomingMessage.senderRank);
  ASSERT_EQ(1, secondIncomingMessage.clock);
  ASSERT_EQ(FirstBroadcast,
            secondIncomingMessage.sessionMessage->msgId);
}

// This tests the behavior of the LCR when a site receives an acknowledgement
// and is expected to return an acknowledgement.
TEST_F(LCRTest, AcknowledgementToAcknowledgement) {
  Arguments arguments{sites};
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  // The original message that was sent (tested before).
  auto firstIncomingMessage{deserializeStruct<MessagePacket>(
      std::move(commLayerRaw->getSent()[0].second))};

  auto message =
      MessagePacket{.messageId = MessageId::Acknowledgement,
                    .senderRank = 2,
                    .clock = 1,
                    .sessionMessage = firstIncomingMessage.sessionMessage};

  ASSERT_EQ(1, commLayerRaw->getSent().size());

  // Send the message to the current site.
  algoLayerRaw->callbackReceive(serializeStruct(message));

  ASSERT_EQ(2, commLayerRaw->getSent().size());

  auto secondIncomingMessage{deserializeStruct<MessagePacket>(
      std::move(commLayerRaw->getSent()[1].second))};
  ASSERT_EQ(MessageId::Acknowledgement, secondIncomingMessage.messageId);
  ASSERT_EQ(2, secondIncomingMessage.senderRank);
  ASSERT_EQ(1, secondIncomingMessage.clock);
  ASSERT_EQ(FirstBroadcast,
            secondIncomingMessage.sessionMessage->msgId);
}

// This tests the behavior of the LCR when a site receives an acknowledgement
// and is expected to return nothing (end of the total order broadcast).
TEST_F(LCRTest, AcknowledgmentToNothing) {
  Arguments arguments{sites};
  SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
  algoLayerRaw->execute();

  // The original message that was sent (tested before).
  auto firstIncomingMessage{deserializeStruct<MessagePacket>(
      std::move(commLayerRaw->getSent()[0].second))};

  auto message =
      MessagePacket{.messageId = MessageId::Acknowledgement,
                    .senderRank = 1,
                    .clock = 1,
                    .sessionMessage = firstIncomingMessage.sessionMessage};

  ASSERT_EQ(1, commLayerRaw->getSent().size());

  // Send the message to the current site.
  algoLayerRaw->callbackReceive(serializeStruct(message));

  ASSERT_EQ(2, commLayerRaw->getSent().size());
}

}  // namespace fbae::test::AlgoLayer
