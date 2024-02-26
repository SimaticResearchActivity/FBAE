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
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, stepMsg.batchesBroadcast[0].batchSessionMsg[0]->msgId);

        // Check @AlgoLayer:broadcastersGroup is correct
        EXPECT_EQ(nbSites, algoLayerRaw->getBroadcastersGroup().size());
        EXPECT_EQ(0, algoLayerRaw->getBroadcastersGroup()[0]);
        EXPECT_EQ(1, algoLayerRaw->getBroadcastersGroup()[1]);
        EXPECT_EQ(2, algoLayerRaw->getBroadcastersGroup()[2]);
        EXPECT_EQ(3, algoLayerRaw->getBroadcastersGroup()[3]);

        // Check Participant is broadcasting messages
        EXPECT_TRUE(algoLayerRaw->isBroadcastingMessages());

        // Check @SessionLayer::callbackInitDone() was called
        EXPECT_TRUE(sessionStub.isCallbackInitDoneCalled());

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
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, stepMsg.batchesBroadcast[0].batchSessionMsg[0]->msgId);

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
        EXPECT_TRUE(sessionStub.isCallbackInitDoneCalled());

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
        auto sessionMsg = make_shared<SessionTest>(SessionMsgId::TestMessage,
                                                   "A");
        algoLayerRaw->totalOrderBroadcast(sessionMsg);

        // Check that no message was sent as message is stored in @AlgoLayer::batchWaitingSessionMsg
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
        // Prepare Step message to be received in current wave from sender 0
        auto sessionMsgA = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                "A");
        auto sessionMsgB = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                "B");
        std::vector<SessionMsg > v{sessionMsgA, sessionMsgB};
        fbae_AlgoLayer::BatchSessionMsg batchSessionMsg {
                0,
                v};
        auto stepMsgAsString {serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step,
                                                                                                         0,
                                                                                                         0,
                                                                                                         0,
                                                                                                         vector<fbae_AlgoLayer::BatchSessionMsg>{batchSessionMsg}})};
        algoLayerRaw->callbackReceive(std::move(stepMsgAsString));

        // Check all messages are delivered in the correct order
        EXPECT_EQ(3, sessionStub.getDelivered().size());
        EXPECT_EQ(0, sessionStub.getDelivered()[0].first);
        EXPECT_EQ(sessionMsgA->msgId, sessionStub.getDelivered()[0].second->msgId);
        auto nakedA = dynamic_cast<SessionTest*>(sessionStub.getDelivered()[0].second.get());
        EXPECT_EQ(sessionMsgA->payload, nakedA->payload);
        EXPECT_EQ(0, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(sessionMsgB->msgId, sessionStub.getDelivered()[1].second->msgId);
        auto nakedB = dynamic_cast<SessionTest*>(sessionStub.getDelivered()[1].second.get());
        EXPECT_EQ(sessionMsgB->payload, nakedB->payload);
        EXPECT_EQ(myRank, sessionStub.getDelivered()[2].first);
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, sessionStub.getDelivered()[2].second->msgId);

        // Check a new Step message has been sent
        EXPECT_EQ(1, commLayerRaw->getSent().size());
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);
        // Check contents of this message
        auto stepMsg{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg.msgId);
        EXPECT_EQ(myRank, stepMsg.senderPos);
        EXPECT_EQ(1, stepMsg.wave);
        EXPECT_EQ(0, stepMsg.step);
        EXPECT_EQ(1, stepMsg.batchesBroadcast.size()); // In current BBOBB implementation, we store batches of messages even though batches of messages is empty
        EXPECT_EQ(myRank, stepMsg.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg.batchesBroadcast[0].batchSessionMsg.size());
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
        // Prepare Step message to be received in current wave from sender 0
        auto sessionMsgC = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                "C");
        auto sessionMsgD = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                "D");
        std::vector<SessionMsg> vWave1{sessionMsgC, sessionMsgD};
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

        // Now prepare Step message to be received in current wave from sender 0
        auto sessionMsgA = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                "A");
        auto sessionMsgB = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                "B");
        std::vector<SessionMsg > v{sessionMsgA, sessionMsgB};
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
        EXPECT_EQ(sessionMsgA->msgId, sessionStub.getDelivered()[0].second->msgId);
        auto nakedA = dynamic_cast<SessionTest*>(sessionStub.getDelivered()[0].second.get());
        EXPECT_EQ(sessionMsgA->payload, nakedA->payload);
        EXPECT_EQ(0, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(sessionMsgB->msgId, sessionStub.getDelivered()[1].second->msgId);
        auto nakedB = dynamic_cast<SessionTest*>(sessionStub.getDelivered()[1].second.get());
        EXPECT_EQ(sessionMsgB->payload, nakedB->payload);
        EXPECT_EQ(myRank, sessionStub.getDelivered()[2].first);
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, sessionStub.getDelivered()[2].second->msgId);
        EXPECT_EQ(0, sessionStub.getDelivered()[3].first);
        EXPECT_EQ(sessionMsgC->msgId, sessionStub.getDelivered()[3].second->msgId);
        auto nakedC = dynamic_cast<SessionTest*>(sessionStub.getDelivered()[3].second.get());
        EXPECT_EQ(sessionMsgC->payload, nakedC->payload);
        EXPECT_EQ(0, sessionStub.getDelivered()[4].first);
        EXPECT_EQ(sessionMsgD->msgId, sessionStub.getDelivered()[4].second->msgId);
        auto nakedD = dynamic_cast<SessionTest*>(sessionStub.getDelivered()[4].second.get());
        EXPECT_EQ(sessionMsgD->payload, nakedD->payload);

        // Check two new Step messages have been sent
        EXPECT_EQ(2, commLayerRaw->getSent().size());
        // Check message 0, i.e. first sent message
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);
        // Check contents of message 0
        auto stepMsg0{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg0.msgId);
        EXPECT_EQ(myRank, stepMsg0.senderPos);
        EXPECT_EQ(1, stepMsg0.wave);
        EXPECT_EQ(0, stepMsg0.step);
        EXPECT_EQ(1, stepMsg0.batchesBroadcast.size()); // In current BBOBB implementation, we store batches of messages even though batches of messages is empty
        EXPECT_EQ(myRank, stepMsg0.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg0.batchesBroadcast[0].batchSessionMsg.size());
        // Check message 1, i.e. second sent message3
        EXPECT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[1].first);
        // Check contents of message 1
        auto stepMsg1{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[1].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg1.msgId);
        EXPECT_EQ(myRank, stepMsg1.senderPos);
        EXPECT_EQ(2, stepMsg1.wave);
        EXPECT_EQ(0, stepMsg1.step);
        EXPECT_EQ(1, stepMsg1.batchesBroadcast.size()); // In current BBOBB implementation, we store batches of messages even though batches of messages is empty
        EXPECT_EQ(myRank, stepMsg1.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg1.batchesBroadcast[0].batchSessionMsg.size());
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
        // Prepare Step message to be received in incorrect wave from sender 0
        auto sessionMsgE = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                "E");
        auto sessionMsgF = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                "E");
        std::vector<SessionMsg> vWave2{sessionMsgE, sessionMsgF};
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
        EXPECT_DEATH({algoLayerRaw->callbackReceive("z_thatIsMessageWithNonSenseMsgId which is 'z'");},
                     ".*Unexpected msgId.*"); // Syntax of matcher is presented at https://google.github.io/googletest/advanced.html#regular-expression-syntax
    }
}