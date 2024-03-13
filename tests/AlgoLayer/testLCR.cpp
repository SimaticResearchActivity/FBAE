//
// Created by simatic on 2/17/24.
//

#include <gtest/gtest.h>

#include "msgTemplates.h"
#include "AlgoLayer/LCR/LCR.h"
#include "AlgoLayer/LCR/LCRMessage.h"

#include "../CommLayer/CommStub.h"
#include "../SessionLayer/SessionStub.h"

namespace fbae_test_LCR {

    using namespace std;
    using namespace fbae_SessionLayer;

    TEST(LCRLayer, LCRLayerExecute) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw = commLayer.get();
        auto algoLayer = make_unique<LCRLayer>(std::move(commLayer));
        auto algoLayerRaw = algoLayer.get();
        rank_t myRank = 3;
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
        auto broadcastMsg{deserializeStruct<fbae_LCRAlgoLayer::StructBroadcastMessage>(std::move(commLayerRaw->getSent()[0].second))};
        ASSERT_EQ(fbae_LCRAlgoLayer::MessageId::Message, broadcastMsg.messageId);
        ASSERT_EQ(3, broadcastMsg.senderRank);
        ASSERT_EQ(std::vector<uint32_t>({ 0, 0, 0, 1 }), broadcastMsg.vectorClock);
        ASSERT_FALSE(broadcastMsg.isStable);
        ASSERT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, broadcastMsg.sessionMessage->msgId);

        // Check plain Participant is broadcasting messages
//        ASSERT_TRUE(algoLayerRaw->isBroadcastingMessages());

        // Check @SessionLayer::callbackInitDone() was called
        ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

        // Check no message was delivered
        ASSERT_EQ(0, sessionStub.getDelivered().size());
    }

//    TEST(Sequencer, TotalOrderBroadcast) {
//        constexpr auto nbSites = 4;
//        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
//        Arguments arguments{sites};
//        auto commLayer = make_unique<CommStub>();
//        auto commLayerRaw= commLayer.get();
//        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
//        auto algoLayerRaw= algoLayer.get();
//        rank_t myRank = sequencerRank + 1;
//        SessionStub sessionStub{arguments, sequencerRank + 1, std::move(algoLayer)};
//
//        algoLayerRaw->execute();
//        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
//        auto sessionMsg = make_shared<SessionTest>(SessionMsgId::TestMessage,
//                                                   "A");
//        algoLayerRaw->totalOrderBroadcast(sessionMsg);
//
//        // Check that 1 message was sent
//        ASSERT_EQ(1, commLayerRaw->getSent().size());
//        // Check that this message was sent to sequencer
//        ASSERT_EQ(sequencerRank, commLayerRaw->getSent()[0].first);
//        // Check contents of this message
//        auto broadcastRequestMsg{deserializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(std::move(commLayerRaw->getSent()[0].second))};
//        ASSERT_EQ(fbae_SequencerAlgoLayer::MsgId::BroadcastRequest, broadcastRequestMsg.msgId);
//        ASSERT_EQ(myRank-1, broadcastRequestMsg.senderPos); // myRank-1 because senderPos is 1 less than senderRank
//        ASSERT_EQ(sessionMsg->msgId, broadcastRequestMsg.sessionMsg->msgId);
//        ASSERT_EQ(sessionMsg->getPayload(), broadcastRequestMsg.sessionMsg->getPayload());
//
//        // Check that no message is delivered
//        ASSERT_EQ(0, sessionStub.getDelivered().size());
//    }
//
//    TEST(Sequencer, SequencerReceivesBroadcastRequest) {
//        constexpr auto nbSites = 4;
//        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
//        Arguments arguments{sites};
//        auto commLayer = make_unique<CommStub>();
//        auto commLayerRaw= commLayer.get();
//        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
//        auto algoLayerRaw= algoLayer.get();
//        auto myRank = sequencerRank;
//        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
//
//        algoLayerRaw->execute();
//        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
//        auto sessionMsg = make_shared<SessionTest>(SessionMsgId::TestMessage,
//                                                   "A");
//        constexpr rank_t senderPos = 42;
//        auto algoMsgAsString {serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(fbae_SequencerAlgoLayer::StructBroadcastMessage{
//            fbae_SequencerAlgoLayer::MsgId::BroadcastRequest,
//            senderPos,
//            sessionMsg})};
//        algoLayerRaw->callbackReceive(std::move(algoMsgAsString));
//
//        // Check that 3 messages are sent by Sequencer
//        ASSERT_EQ(nbSites-1, commLayerRaw->getSent().size());
//        // Check that destinations of all of these messages
//        ASSERT_EQ(1, commLayerRaw->getSent()[0].first);
//        ASSERT_EQ(2, commLayerRaw->getSent()[1].first);
//        ASSERT_EQ(3, commLayerRaw->getSent()[2].first);
//        // Check second and third messages are equal to first message.
//        ASSERT_EQ(commLayerRaw->getSent()[0].second, commLayerRaw->getSent()[1].second);
//        ASSERT_EQ(commLayerRaw->getSent()[0].second, commLayerRaw->getSent()[2].second);
//        // Check contents of the first message
//        auto broadcastMsg{deserializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(std::move(commLayerRaw->getSent()[0].second))};
//        ASSERT_EQ(fbae_SequencerAlgoLayer::MsgId::Broadcast, broadcastMsg.msgId);
//        ASSERT_EQ(senderPos, broadcastMsg.senderPos);
//        ASSERT_EQ(sessionMsg->msgId, broadcastMsg.sessionMsg->msgId);
//        ASSERT_EQ(sessionMsg->getPayload(), broadcastMsg.sessionMsg->getPayload());
//
//        // Check that no message is delivered
//        ASSERT_EQ(0, sessionStub.getDelivered().size());
//    }
//
//    TEST(Sequencer, ParticipantReceivesBroadcast) {
//        constexpr auto nbSites = 4;
//        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
//        Arguments arguments{sites};
//        auto commLayer = make_unique<CommStub>();
//        auto commLayerRaw = commLayer.get();
//        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
//        auto algoLayerRaw= algoLayer.get();
//        rank_t myRank = sequencerRank + 1;
//        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
//
//        algoLayerRaw->execute();
//        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
//        auto sessionMsg = make_shared<SessionTest>(SessionMsgId::TestMessage,
//                                                   "A");
//        constexpr rank_t senderPos = 42;
//        auto algoMsgAsString {serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(fbae_SequencerAlgoLayer::StructBroadcastMessage{
//            fbae_SequencerAlgoLayer::MsgId::Broadcast,
//            senderPos,
//            sessionMsg})};
//        algoLayerRaw->callbackReceive(std::move(algoMsgAsString));
//
//        // Check that no message has been sent
//        ASSERT_EQ(0, commLayerRaw->getSent().size());
//
//        // Check that 1 message has been delivered
//        ASSERT_EQ(1, sessionStub.getDelivered().size());
//        // Check sender of this message
//        ASSERT_EQ(senderPos, sessionStub.getDelivered()[0].first);
//        // Check contents of this message
//        ASSERT_EQ(sessionMsg->msgId, sessionStub.getDelivered()[0].second->msgId);
//        ASSERT_EQ(sessionMsg->getPayload(), sessionStub.getDelivered()[0].second->getPayload());
//    }
//
//    TEST(Sequencer, ParticipantReceivesUnknownMessage) {
//        constexpr auto nbSites = 4;
//        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
//        Arguments arguments{sites};
//        auto commLayer = make_unique<CommStub>();
//        auto commLayerRaw = commLayer.get();
//        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
//        auto algoLayerRaw= algoLayer.get();
//        rank_t myRank = sequencerRank + 1;
//        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};
//
//        algoLayerRaw->execute();
//        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
//        ASSERT_DEATH({algoLayerRaw->callbackReceive("z_thatIsMessageWithNonSenseMsgId which is 'z'");},
//                     ".*UnASSERTed msgId.*"); // Syntax of matcher is presented at https://google.github.io/googletest/advanced.html#regular-expression-syntax
//    }
}
