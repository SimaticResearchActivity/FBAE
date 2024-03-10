//
// Created by simatic on 2/18/24.
//

#include <gtest/gtest.h>

#include "msgTemplates.h"
#include "AlgoLayer/BBOBB/BBOBB.h"

#include "../CommLayer/CommStub.h"
#include "../SessionLayer/SessionStub.h"

namespace fbae_test_BBOBB {

    using namespace std;
    using namespace fbae_SessionLayer;

    auto constexpr payloadA{"A"};
    auto constexpr payloadB{"B"};
    auto constexpr payloadC{"C"};
    auto constexpr payloadD{"D"};

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

    static string buildStepMsg (rank_t stepSender, uint8_t wave, uint8_t step,
                                vector<fbae_AlgoLayer::BatchSessionMsg> const& v) {
        return serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step,
                                                                                          stepSender,
                                                                                          wave,
                                                                                          step,
                                                                                          v});

    }

    static string buildStepMsg (rank_t stepSender, uint8_t wave, uint8_t step,
                                rank_t batchSender, string const& payload) {
        auto sessionMsg = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payload);
        fbae_AlgoLayer::BatchSessionMsg batch {
                batchSender,
                vector<SessionMsg>{sessionMsg}};

        return buildStepMsg(stepSender, wave, step,
                            vector<fbae_AlgoLayer::BatchSessionMsg>{batch});
    }

    static string buildStepMsg (rank_t stepSender, uint8_t wave, uint8_t step,
                                rank_t batchSender, string const& payload1, string const& payload2) {
        auto sessionMsg1 = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payload1);
        auto sessionMsg2 = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payload2);
        fbae_AlgoLayer::BatchSessionMsg batch {
                batchSender,
                vector<SessionMsg>{sessionMsg1, sessionMsg2}};

        return buildStepMsg(stepSender, wave, step,
                            vector<fbae_AlgoLayer::BatchSessionMsg>{batch});
    }

    static string buildStepMsg (rank_t stepSender, uint8_t wave, uint8_t step,
                                rank_t batch1Sender, string const& payload1,
                                rank_t batch2Sender, string const& payload2) {
        auto sessionMsg1 = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payload1);
        fbae_AlgoLayer::BatchSessionMsg batch1 {
                batch1Sender,
                vector<SessionMsg>{sessionMsg1}};

        auto sessionMsg2 = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payload2);
        fbae_AlgoLayer::BatchSessionMsg batch2 {
                batch2Sender,
                vector<SessionMsg>{sessionMsg2}};

        return buildStepMsg(stepSender, wave, step,
                            vector<fbae_AlgoLayer::BatchSessionMsg>{batch1, batch2});
    }

    static string buildComplexStepMsg (rank_t stepSender, uint8_t wave, uint8_t step,
                                rank_t batch1Sender,
                                rank_t batch2Sender,
                                rank_t batch3Sender) {
        auto sessionMsg11 = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payloadA);
        auto sessionMsg12 = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payloadB);
        fbae_AlgoLayer::BatchSessionMsg batch1 {
                batch1Sender,
                vector<SessionMsg>{sessionMsg11, sessionMsg12}};

        auto sessionMsg2 = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payloadD);
        fbae_AlgoLayer::BatchSessionMsg batch2 {
                batch2Sender,
                vector<SessionMsg>{sessionMsg2}};

        auto sessionMsg3 = make_shared<SessionTest>(
                SessionMsgId::TestMessage,
                payloadC);
        fbae_AlgoLayer::BatchSessionMsg batch3{
                batch3Sender,
                vector<SessionMsg>{sessionMsg3}};

        return buildStepMsg(stepSender, wave, step,
                            vector<fbae_AlgoLayer::BatchSessionMsg>{batch1, batch2, batch3});
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
        algoLayerRaw->callbackReceive(buildStepMsg(0, 0, 0,
                                                   0, payloadA, payloadB));

        // Check all messages are delivered in the correct order
        EXPECT_EQ(3, sessionStub.getDelivered().size());
        EXPECT_EQ(0, sessionStub.getDelivered()[0].first);
        EXPECT_EQ(SessionMsgId::TestMessage, sessionStub.getDelivered()[0].second->msgId);
        EXPECT_EQ(payloadA, sessionStub.getDelivered()[0].second->getPayload());
        EXPECT_EQ(0, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(SessionMsgId::TestMessage, sessionStub.getDelivered()[1].second->msgId);
        EXPECT_EQ(payloadB, sessionStub.getDelivered()[1].second->getPayload());
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
        // Prepare Step message to be received in next wave from sender 0
        algoLayerRaw->callbackReceive(buildStepMsg(0, 1, 0,
                                                   0, payloadC, payloadD));

        // Check that no message was sent, nor delivered
        EXPECT_EQ(0, commLayerRaw->getSent().size());
        EXPECT_EQ(0, sessionStub.getDelivered().size());

        // Now prepare Step message to be received in current wave from sender 0
        algoLayerRaw->callbackReceive(buildStepMsg(0, 0, 0,
                                                   0, payloadA, payloadB));

        // Check all messages are delivered in the correct order
        EXPECT_EQ(5, sessionStub.getDelivered().size());
        EXPECT_EQ(0, sessionStub.getDelivered()[0].first);
        EXPECT_EQ(payloadA, sessionStub.getDelivered()[0].second->getPayload()); // We do not care about msgId as it was checked in another test + It must be as TestMessage to behave correctly with getPayload()
        EXPECT_EQ(0, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(payloadB, sessionStub.getDelivered()[1].second->getPayload());
        EXPECT_EQ(myRank, sessionStub.getDelivered()[2].first);
        EXPECT_EQ(fbae_SessionLayer::SessionMsgId::FirstBroadcast, sessionStub.getDelivered()[2].second->msgId);
        EXPECT_EQ(0, sessionStub.getDelivered()[3].first);
        EXPECT_EQ(payloadC, sessionStub.getDelivered()[3].second->getPayload());
        EXPECT_EQ(0, sessionStub.getDelivered()[4].first);
        EXPECT_EQ(payloadD, sessionStub.getDelivered()[4].second->getPayload());

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

    TEST(BBOBB, ExecuteWith8SitesAndRank1ReceiveStepsWhichDoNotTriggerImmediatlySendingOfmessages) {
        /*
         * In this scenario, site with rank 1:
         *   0. because of initialization, has sent step 0 message to 2 in wave 0 (with SessionMsg FirstBroadcast from 1)
         *      Note: This was already verified in previous scenario.
         *   1. receives step 2 message from 5 in wave 0 (with SessionMsg 'A' and 'B' from sender 0 & SessionMsg 'D' from sender 5 & SessionMsg 'C' from sender 3)
         *      ==> No reaction of site with rank 1
         *   2. receives step 0 message from 0 in wave 1 (with SessionMsg 'E' from sender 0)
         *      ==> No reaction of site with rank 1
         *   3. receives step 1 message for 7 in wave 1 (with SessionMsg 'F' from sender 7)
         *      ==> No reaction of site with rank 1
         *   4. receives step 0 message from 0 in wave 0 (with SessionMsg 'A' and 'B' from sender 0)
         *      ==> Site with rank 1 sends step 1 message to 3 in wave 0 (with SessionMsg FirstBroadcast from 1 & Session Msg 'A' and 'B' from 0)
         *   5. receives step 1 message from 7 in wave 0 (with SessionMsg 'G' from sender 7 & SessionMsg 'D' from sender 5)
         *      ==> Site with rank 1
         *           a. sends step 2 message in wave 0 to 5 (with SessionMsg FirstBroadcast from 1 & SessionMsg 'G' from 1 & SessionMsg 'A' and 'B' from 0 & SessionMsg 'A' and 'B' from sender 0 & SessionMsg 'G' from sender 7 & SessionMsg 'D' from sender 5)
         *           b. calls deliver for wave 0
         *              ==> SessionMsg 'A' and 'B' from 0, SessionMsg FirstBroadcast from 1, SessionMsg 'C' from sender 3, SessionMsg 'D' from sender 5, SessionMsg 'G' from sender 7
         *           c. sends step 0 message to 2 in wave 1 (empty)
         *           d. sends step 1 message to 3 in wave 1 (SessionMsg 'E' from sender 0)
         *           e. sends step 2 message to 5 in wave 1 (SessionMsg 'E' from sender 0 & SessionMsg 'F' from sender 7)
         *           f. does not call deliver for wave 1 (as it is missing step 2 message of wave 1)
         */
        constexpr auto nbSites = 8;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<BBOBB>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        rank_t myRank = 1;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear information concerning sending of FirstBroadcast which we already tested in previous execute() tests
        // 1. receives step 2 message from 5 in wave 0 (with SessionMsg 'A' and 'B' from sender 0
        //                                                   & SessionMsg 'D' from sender 5
        //                                                   & SessionMsg 'C' from sender 3)
        //    ==> No reaction of site with rank 1
        algoLayerRaw->callbackReceive( buildComplexStepMsg( 5, 0, 2,
                                                     0, /* payloadA, payloadB, */
                                                     5, /* payloadD, */
                                                     3  /* payloadC */ ) );

        EXPECT_EQ(0, commLayerRaw->getSent().size());
        EXPECT_EQ(0, sessionStub.getDelivered().size());

        // 2. receives step 0 message from 0 in wave 1 (with SessionMsg 'E' from sender 0)
        //    ==> No reaction of site with rank 1
        auto constexpr payloadE{"E"};
        algoLayerRaw->callbackReceive( buildStepMsg( 0, 1, 0,
                                                     0, payloadE) );

        EXPECT_EQ(0, commLayerRaw->getSent().size());
        EXPECT_EQ(0, sessionStub.getDelivered().size());

        // 3. receives step 1 message for 7 in wave 1 (with SessionMsg 'F' from sender 7)
        //    ==> No reaction of site with rank 1
        auto constexpr payloadF{"F"};
        algoLayerRaw->callbackReceive( buildStepMsg( 7, 1, 1,
                                                     7, payloadF) );

        EXPECT_EQ(0, commLayerRaw->getSent().size());
        EXPECT_EQ(0, sessionStub.getDelivered().size());

        // 4. receives step 0 message from 0 in wave 0 (with SessionMsg 'A' and 'B' from sender 0)
        //    ==> Site with rank 1 sends step 1 message to 3 in wave 0 (with SessionMsg FirstBroadcast from 1 & Session Msg 'A' and 'B' from 0)
        algoLayerRaw->callbackReceive( buildStepMsg( 0, 0, 0,
                                                     0, payloadA, payloadB) );

        EXPECT_EQ(3, commLayerRaw->getSent()[0].first);
        auto stepMsg0{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg0.msgId);
        EXPECT_EQ(myRank, stepMsg0.senderPos);
        EXPECT_EQ(0, stepMsg0.wave);
        EXPECT_EQ(1, stepMsg0.step);
        EXPECT_EQ(2, stepMsg0.batchesBroadcast.size()); // In current BBOBB implementation, we store batches of messages even though batches of messages is empty

        EXPECT_EQ(myRank, stepMsg0.batchesBroadcast[0].senderPos);
        EXPECT_EQ(1, stepMsg0.batchesBroadcast[0].batchSessionMsg.size());
        EXPECT_EQ(SessionMsgId::FirstBroadcast, stepMsg0.batchesBroadcast[0].batchSessionMsg[0]->msgId);

        EXPECT_EQ(0, stepMsg0.batchesBroadcast[1].senderPos);
        EXPECT_EQ(2, stepMsg0.batchesBroadcast[1].batchSessionMsg.size());
        EXPECT_EQ(payloadA, stepMsg0.batchesBroadcast[1].batchSessionMsg[0]->getPayload());
        EXPECT_EQ(payloadB, stepMsg0.batchesBroadcast[1].batchSessionMsg[1]->getPayload());

        // 5. receives step 1 message from 7 in wave 0 (with SessionMsg 'G' from sender 7 & SessionMsg 'D' from sender 5)
        //    ==> Site with rank 1
        //          a. sends step 2 message in wave 0 to 5 (with SessionMsg FirstBroadcast from 1 & SessionMsg 'G' from 1 & SessionMsg 'A' and 'B' from 0 & SessionMsg 'A' and 'B' from sender 0 & SessionMsg 'G' from sender 7 & SessionMsg 'D' from sender 3)
        //          b. calls deliver for wave 0
        //             ==> SessionMsg 'A' and 'B' from 0, SessionMsg FirstBroadcast from 1, SessionMsg 'C' from sender 3, SessionMsg 'D' from sender 5, SessionMsg 'G' from sender 7
        //          c. sends step 0 message to 2 in wave 1 (empty batch from sender 1)
        //          d. sends step 1 message to 3 in wave 1 (empty batch from sender 1 & SessionMsg 'E' from sender 0)
        //          e. sends step 2 message to 5 in wave 1 (empty batch from sender 1 & SessionMsg 'E' from sender 0 & SessionMsg 'F' from sender 7)
        //          f. does not call deliver for wave 1 (as it is missing step 2 message of wave 1)
        auto constexpr payloadG{"G"};
        algoLayerRaw->callbackReceive( buildStepMsg( 7, 0, 1,
                                                     7, payloadG,
                                                     5, payloadD) );

        // Check a. sends step 2 message in wave 0 to 5 (with SessionMsg FirstBroadcast from 1
        //                                              & SessionMsg 'A' and 'B' from 0
        //                                              & SessionMsg 'G' from 7
        //                                              & SessionMsg 'D' from sender 5)
        EXPECT_EQ(5, commLayerRaw->getSent()[1].first);
        auto stepMsg02{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[1].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg02.msgId);
        EXPECT_EQ(myRank, stepMsg02.senderPos);
        EXPECT_EQ(0, stepMsg02.wave);
        EXPECT_EQ(2, stepMsg02.step);
        EXPECT_EQ(4, stepMsg02.batchesBroadcast.size());

        EXPECT_EQ(myRank, stepMsg02.batchesBroadcast[0].senderPos);
        EXPECT_EQ(1, stepMsg02.batchesBroadcast[0].batchSessionMsg.size());
        EXPECT_EQ(SessionMsgId::FirstBroadcast, stepMsg02.batchesBroadcast[0].batchSessionMsg[0]->msgId);

        EXPECT_EQ(0, stepMsg02.batchesBroadcast[1].senderPos);
        EXPECT_EQ(2, stepMsg02.batchesBroadcast[1].batchSessionMsg.size());
        EXPECT_EQ(payloadA, stepMsg02.batchesBroadcast[1].batchSessionMsg[0]->getPayload());
        EXPECT_EQ(payloadB, stepMsg02.batchesBroadcast[1].batchSessionMsg[1]->getPayload());

        EXPECT_EQ(7, stepMsg02.batchesBroadcast[2].senderPos);
        EXPECT_EQ(1, stepMsg02.batchesBroadcast[2].batchSessionMsg.size());
        EXPECT_EQ(payloadG, stepMsg02.batchesBroadcast[2].batchSessionMsg[0]->getPayload());

        EXPECT_EQ(5, stepMsg02.batchesBroadcast[3].senderPos);
        EXPECT_EQ(1, stepMsg02.batchesBroadcast[3].batchSessionMsg.size());
        EXPECT_EQ(payloadD, stepMsg02.batchesBroadcast[3].batchSessionMsg[0]->getPayload());

        // Check b. calls deliver for wave 0
        //          ==> SessionMsg 'A' and 'B' from 0,
        //             SessionMsg FirstBroadcast from 1,
        //             SessionMsg 'C' from sender 3,
        //             SessionMsg 'D' from sender 5,
        //             SessionMsg 'G' from sender 7
        //       f. As only 6 messages are delivered, does not call deliver for wave 1 (as it is missing step 2 message of wave 1)
        EXPECT_EQ(6, sessionStub.getDelivered().size());

        EXPECT_EQ(0, sessionStub.getDelivered()[0].first);
        EXPECT_EQ(payloadA, sessionStub.getDelivered()[0].second->getPayload());

        EXPECT_EQ(0, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(payloadB, sessionStub.getDelivered()[1].second->getPayload());

        EXPECT_EQ(myRank, sessionStub.getDelivered()[2].first);
        EXPECT_EQ(SessionMsgId::FirstBroadcast, sessionStub.getDelivered()[2].second->msgId);

        EXPECT_EQ(3, sessionStub.getDelivered()[3].first);
        EXPECT_EQ(payloadC, sessionStub.getDelivered()[3].second->getPayload());

        EXPECT_EQ(5, sessionStub.getDelivered()[4].first);
        EXPECT_EQ(payloadD, sessionStub.getDelivered()[4].second->getPayload());

        EXPECT_EQ(7, sessionStub.getDelivered()[5].first);
        EXPECT_EQ(payloadG, sessionStub.getDelivered()[5].second->getPayload());

        // Check c. sends step 0 message to 2 in wave 1 (empty batch from 1)
        EXPECT_EQ(2, commLayerRaw->getSent()[2].first);
        auto stepMsg10{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[2].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg10.msgId);
        EXPECT_EQ(myRank, stepMsg10.senderPos);
        EXPECT_EQ(1, stepMsg10.wave);
        EXPECT_EQ(0, stepMsg10.step);
        EXPECT_EQ(1, stepMsg10.batchesBroadcast.size()); // In current BBOBB implementation, we store batches of messages even though batches of messages is empty

        EXPECT_EQ(myRank, stepMsg10.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg10.batchesBroadcast[0].batchSessionMsg.size());

        // Check d. sends step 1 message to 3 in wave 1 (empty batch from sender 1
        //                                               & SessionMsg 'E' from sender 0)
        EXPECT_EQ(3, commLayerRaw->getSent()[3].first);
        auto stepMsg11{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[3].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg11.msgId);
        EXPECT_EQ(myRank, stepMsg11.senderPos);
        EXPECT_EQ(1, stepMsg11.wave);
        EXPECT_EQ(1, stepMsg11.step);
        EXPECT_EQ(2, stepMsg11.batchesBroadcast.size());

        EXPECT_EQ(myRank, stepMsg10.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg10.batchesBroadcast[0].batchSessionMsg.size());

        EXPECT_EQ(0, stepMsg11.batchesBroadcast[1].senderPos);
        EXPECT_EQ(1, stepMsg11.batchesBroadcast[1].batchSessionMsg.size());
        EXPECT_EQ(payloadE, stepMsg11.batchesBroadcast[1].batchSessionMsg[0]->getPayload());

        // Check e. sends step 2 message to 5 in wave 1 (empty batch from sender 1
        //                                               & SessionMsg 'E' from sender 0
        //                                               & SessionMsg 'F' from sender 7)
        EXPECT_EQ(5, commLayerRaw->getSent()[4].first);
        auto stepMsg12{deserializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(std::move(commLayerRaw->getSent()[4].second))};
        EXPECT_EQ(fbae_BBOBBAlgoLayer::MsgId::Step, stepMsg12.msgId);
        EXPECT_EQ(myRank, stepMsg12.senderPos);
        EXPECT_EQ(1, stepMsg12.wave);
        EXPECT_EQ(2, stepMsg12.step);
        EXPECT_EQ(3, stepMsg12.batchesBroadcast.size());

        EXPECT_EQ(myRank, stepMsg10.batchesBroadcast[0].senderPos);
        EXPECT_EQ(0, stepMsg10.batchesBroadcast[0].batchSessionMsg.size());

        EXPECT_EQ(0, stepMsg12.batchesBroadcast[1].senderPos);
        EXPECT_EQ(1, stepMsg12.batchesBroadcast[1].batchSessionMsg.size());
        EXPECT_EQ(payloadE, stepMsg12.batchesBroadcast[1].batchSessionMsg[0]->getPayload());

        EXPECT_EQ(7, stepMsg12.batchesBroadcast[2].senderPos);
        EXPECT_EQ(1, stepMsg12.batchesBroadcast[2].batchSessionMsg.size());
        EXPECT_EQ(payloadF, stepMsg12.batchesBroadcast[2].batchSessionMsg[0]->getPayload());
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
