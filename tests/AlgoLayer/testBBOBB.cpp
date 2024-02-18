//
// Created by simatic on 2/18/24.
//

#include <gtest/gtest.h>
#include "../../src/msgTemplates.h"
#include "../../src/AlgoLayer/BBOBB/BBOBB.h"
#include "../../src/SessionLayer/SessionLayerMsg.h"
#include "../CommLayer/CommStub.h"
#include "../SessionLayer/SessionStub.h"

namespace fbae_test_BBOBB {

    using namespace std;
    using namespace fbae_SessionLayer;

    TEST(BBOBB, ExecuteWith4SitesAndRank0) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<BBOBB>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        rank_t myRank = 0;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

        algoLayerRaw->execute();

        // Check established connections
        EXPECT_EQ(2, commLayerRaw->getConnectedDest().size());
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getConnectedDest()[0]);
        EXPECT_EQ((myRank + 2) % nbSites, commLayerRaw->getConnectedDest()[1]);

        // Check nbAwaitedConnections
        EXPECT_EQ(2, commLayerRaw->getNbAwaitedConnections());

        // Check FirstBroadcast message was broadcast and thus sent.
        EXPECT_EQ(1, commLayerRaw->getSent().size());
        // Check that this message was sent to successor
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);
        // Check contents of this message
        auto stepMsg{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg.msgId);
        EXPECT_EQ(myRank, stepMsg.senderPos);
        EXPECT_EQ(0, stepMsg.wave);
        EXPECT_EQ(0, stepMsg.step);
        EXPECT_EQ(1, stepMsg.batchesBroadcast.size()); // Just 1 batch message
        EXPECT_EQ(myRank, stepMsg.batchesBroadcast[0].senderPos); // Sender of this batch messages is 0
        auto msgSession{deserializeStruct<fbae_SessionLayer::SessionFirstBroadcast>(std::move(stepMsg.batchesBroadcast[0].batchSessionMsg[0]))};
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, msgSession.msgId);

        // Check @AlgoLayer:broadcastersGroup is correct
        EXPECT_EQ(nbSites, algoLayerRaw->getBroadcastersGroup().size());
        EXPECT_EQ(0, algoLayerRaw->getBroadcastersGroup()[0]);
        EXPECT_EQ(1, algoLayerRaw->getBroadcastersGroup()[1]);
        EXPECT_EQ(2, algoLayerRaw->getBroadcastersGroup()[2]);
        EXPECT_EQ(3, algoLayerRaw->getBroadcastersGroup()[3]);

        // Check Participant is broadcasting messages
        EXPECT_TRUE(algoLayerRaw->isBroadcastingMessages());

        // Check @SessionLayer::callbackInitDone() was called
        EXPECT_TRUE(sessionStub.isCallbackInitdoneCalled());

        // Check no message was delivered
        EXPECT_EQ(0, sessionStub.getDelivered().size());
    }

    TEST(BBOBB, ExecuteWith9SitesAndRank8) {
        constexpr auto nbSites = 9;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<BBOBB>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        rank_t myRank = 8;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

        algoLayerRaw->execute();

        // Check established connections
        EXPECT_EQ(4, commLayerRaw->getConnectedDest().size());
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getConnectedDest()[0]);
        EXPECT_EQ((myRank + 2) % nbSites, commLayerRaw->getConnectedDest()[1]);
        EXPECT_EQ((myRank + 4) % nbSites, commLayerRaw->getConnectedDest()[2]);
        EXPECT_EQ((myRank + 8) % nbSites, commLayerRaw->getConnectedDest()[3]);

        // Check nbAwaitedConnections
        EXPECT_EQ(4, commLayerRaw->getNbAwaitedConnections());

        // Check FirstBroadcast message was broadcast and thus sent.
        EXPECT_EQ(1, commLayerRaw->getSent().size());
        // Check that this message was sent to successor
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);
        // Check contents of this message
        auto stepMsg{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg.msgId);
        EXPECT_EQ(myRank, stepMsg.senderPos);
        EXPECT_EQ(0, stepMsg.wave);
        EXPECT_EQ(0, stepMsg.step);
        EXPECT_EQ(1, stepMsg.batchesBroadcast.size()); // Just 1 batch message
        EXPECT_EQ(myRank, stepMsg.batchesBroadcast[0].senderPos); // Sender of this batch messages is 0
        auto msgSession{deserializeStruct<fbae_SessionLayer::SessionFirstBroadcast>(std::move(stepMsg.batchesBroadcast[0].batchSessionMsg[0]))};
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, msgSession.msgId);

        // Check @AlgoLayer:broadcastersGroup is correct
        EXPECT_EQ(nbSites, algoLayerRaw->getBroadcastersGroup().size());
        EXPECT_EQ(0, algoLayerRaw->getBroadcastersGroup()[0]);
        EXPECT_EQ(1, algoLayerRaw->getBroadcastersGroup()[1]);
        EXPECT_EQ(2, algoLayerRaw->getBroadcastersGroup()[2]);
        EXPECT_EQ(3, algoLayerRaw->getBroadcastersGroup()[3]);
        EXPECT_EQ(4, algoLayerRaw->getBroadcastersGroup()[4]);
        EXPECT_EQ(5, algoLayerRaw->getBroadcastersGroup()[5]);
        EXPECT_EQ(6, algoLayerRaw->getBroadcastersGroup()[6]);
        EXPECT_EQ(7, algoLayerRaw->getBroadcastersGroup()[7]);
        EXPECT_EQ(8, algoLayerRaw->getBroadcastersGroup()[8]);

        // Check Participant is broadcasting messages
        EXPECT_TRUE(algoLayerRaw->isBroadcastingMessages());

        // Check @SessionLayer::callbackInitDone() was called
        EXPECT_TRUE(sessionStub.isCallbackInitdoneCalled());

        // Check no message was delivered
        EXPECT_EQ(0, sessionStub.getDelivered().size());
    }

    TEST(BBOBB, TotalOrderBroadcast) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<BBOBB>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        rank_t myRank = nbSites - 1;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
        constexpr auto payload{"A"};
        auto s {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                         payload})};
        auto sCopy{s};
        algoLayerRaw->totalOrderBroadcast(std::move(s));

        // Check that no message was sent as message is stored in @AlgoLayer::msgsWaitingToBeBroadcast
        EXPECT_EQ(0, commLayerRaw->getSent().size());

        // Check that no message is delivered
        EXPECT_EQ(0, sessionStub.getDelivered().size());
    }

    TEST(BBOBB, ExecuteWith2SitesAndRank1ReceiveStepInWave) {
        constexpr auto nbSites = 2;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<BBOBB>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        rank_t myRank = 1;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
        // Receive Step message in current wave from 0
        auto sessionMsgA {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                         "A"})};
        auto sessionMsgB {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                   "B"})};
        std::vector<std::string> v{sessionMsgA, sessionMsgB};
        fbae_AlgoLayer::BatchSessionMsg batchSessionMsg {
                0,
                v};
        auto sStep {serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step,
                                                                                                   0,
                                                                                                   0,
                                                                                                   0,
                                                                                                   vector<fbae_AlgoLayer::BatchSessionMsg>{batchSessionMsg}})};
        algoLayerRaw->callbackReceive(std::move(sStep));

        // Check all messages are delivered in the correct order
        EXPECT_EQ(3, sessionStub.getDelivered().size());
        EXPECT_EQ(0, sessionStub.getDelivered()[0].first);
        EXPECT_EQ(sessionMsgA, sessionStub.getDelivered()[0].second);
        EXPECT_EQ(0, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(sessionMsgB, sessionStub.getDelivered()[1].second);
        EXPECT_EQ(myRank, sessionStub.getDelivered()[2].first);
        auto msgSession{deserializeStruct<fbae_SessionLayer::SessionFirstBroadcast>(std::move(sessionStub.getDelivered()[2].second))};
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, msgSession.msgId);

        // Check a new Step message has been sent
        EXPECT_EQ(1, commLayerRaw->getSent().size());
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);
        // Check contents of this message
        auto stepMsg2{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg2.msgId);
        EXPECT_EQ(myRank, stepMsg2.senderPos);
        EXPECT_EQ(1, stepMsg2.wave);
        EXPECT_EQ(0, stepMsg2.step);
        EXPECT_EQ(1, stepMsg2.batchesBroadcast.size()); // In current BBOBB implementation, we store batches of messages even though batches of messages is empty
        EXPECT_EQ(myRank, stepMsg2.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg2.batchesBroadcast[0].batchSessionMsg.size());
    }

    TEST(BBOBB, ExecuteWith2SitesAndRank1ReceiveStepInNextWaveThenStepInWave) {
        constexpr auto nbSites = 2;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<BBOBB>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        rank_t myRank = 1;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
        // Receive Step message in next wave from 0
        auto sessionMsgC {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                   "C"})};
        auto sessionMsgD {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                   "D"})};
        std::vector<std::string> vWave1{sessionMsgC, sessionMsgD};
        fbae_AlgoLayer::BatchSessionMsg batchSessionMsgWave1 {
                0,
                vWave1};
        auto sStepWave1 {serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step,
                                                                                               0,
                                                                                               1,
                                                                                               0,
                                                                                               vector<fbae_AlgoLayer::BatchSessionMsg>{batchSessionMsgWave1}})};
        algoLayerRaw->callbackReceive(std::move(sStepWave1));

        // Check that no message was sent, nor delivered
        EXPECT_EQ(0, commLayerRaw->getSent().size());
        EXPECT_EQ(0, sessionStub.getDelivered().size());

        // Receive Step message in current wave from 0
        auto sessionMsgA {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                   "A"})};
        auto sessionMsgB {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                   "B"})};
        std::vector<std::string> v{sessionMsgA, sessionMsgB};
        fbae_AlgoLayer::BatchSessionMsg batchSessionMsg {
                0,
                v};
        auto sStep {serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step,
                                                                                               0,
                                                                                               0,
                                                                                               0,
                                                                                               vector<fbae_AlgoLayer::BatchSessionMsg>{batchSessionMsg}})};
        algoLayerRaw->callbackReceive(std::move(sStep));

        // Check all messages are delivered in the correct order
        EXPECT_EQ(5, sessionStub.getDelivered().size());
        EXPECT_EQ(0, sessionStub.getDelivered()[0].first);
        EXPECT_EQ(sessionMsgA, sessionStub.getDelivered()[0].second);
        EXPECT_EQ(0, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(sessionMsgB, sessionStub.getDelivered()[1].second);
        EXPECT_EQ(myRank, sessionStub.getDelivered()[2].first);
        auto msgSession{deserializeStruct<fbae_SessionLayer::SessionFirstBroadcast>(std::move(sessionStub.getDelivered()[2].second))};
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, msgSession.msgId);
        EXPECT_EQ(0, sessionStub.getDelivered()[3].first);
        EXPECT_EQ(sessionMsgC, sessionStub.getDelivered()[3].second);
        EXPECT_EQ(0, sessionStub.getDelivered()[4].first);
        EXPECT_EQ(sessionMsgD, sessionStub.getDelivered()[4].second);

        // Check 2 new Step messages have been sent
        EXPECT_EQ(2, commLayerRaw->getSent().size());
        // Check message 0
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);
        // Check contents of message 0
        auto stepMsg2{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg2.msgId);
        EXPECT_EQ(myRank, stepMsg2.senderPos);
        EXPECT_EQ(1, stepMsg2.wave);
        EXPECT_EQ(0, stepMsg2.step);
        EXPECT_EQ(1, stepMsg2.batchesBroadcast.size()); // In current BBOBB implementation, we store batches of messages even though batches of messages is empty
        EXPECT_EQ(myRank, stepMsg2.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg2.batchesBroadcast[0].batchSessionMsg.size());
        // Check message 1
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[1].first);
        // Check contents of message 1
        auto stepMsg3{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[1].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg3.msgId);
        EXPECT_EQ(myRank, stepMsg3.senderPos);
        EXPECT_EQ(2, stepMsg3.wave);
        EXPECT_EQ(0, stepMsg3.step);
        EXPECT_EQ(1, stepMsg3.batchesBroadcast.size()); // In current BBOBB implementation, we store batches of messages even though batches of messages is empty
        EXPECT_EQ(myRank, stepMsg3.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg3.batchesBroadcast[0].batchSessionMsg.size());
    }
    TEST(BBOBB, ExecuteWith2SitesAndRank1ReceiveStepInIncorrectWave) {
        constexpr auto nbSites = 2;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<BBOBB>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        rank_t myRank = 1;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
        // Receive Step message in incorrect wave from 0
        auto sessionMsgE {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                   "E"})};
        auto sessionMsgF {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                   "F"})};
        std::vector<std::string> vWave2{sessionMsgE, sessionMsgF};
        fbae_AlgoLayer::BatchSessionMsg batchSessionMsgWave2 {
                0,
                vWave2};
        auto sStepWave2 {serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step,
                                                                                                    0,
                                                                                                    2,
                                                                                                    0,
                                                                                                    vector<fbae_AlgoLayer::BatchSessionMsg>{batchSessionMsgWave2}})};
        EXPECT_DEATH({algoLayerRaw->callbackReceive(std::move(sStepWave2));},
                     ".*Unexpected wave.*"); // Syntax of matcher is presented at https://google.github.io/googletest/advanced.html#regular-expression-syntax
    }

    TEST(BBOBB, ExecuteWith2SitesAndRank1ReceiveUnknownMessage) {
        constexpr auto nbSites = 2;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<BBOBB>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        rank_t myRank = 1;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
        EXPECT_DEATH({algoLayerRaw->callbackReceive("z_thatIsMessageWithNonSenseMsgId = 'z'");},
                     ".*Unexpected msgId.*"); // Syntax of matcher is presented at https://google.github.io/googletest/advanced.html#regular-expression-syntax
    }
}