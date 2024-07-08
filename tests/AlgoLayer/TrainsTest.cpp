#include <gtest/gtest.h>

#include "msgTemplates.h"
#include "AlgoLayer/Trains/Trains.h"

#include "../CommLayer/CommStub.h"
#include "../SessionLayer/SessionStub.h"

namespace fbae_test_Trains {

    using namespace std;
    using namespace fbae_SessionLayer;

    auto constexpr payloadA{ "A" };
    auto constexpr payloadB{ "B" };
    auto constexpr payloadC{ "C" };
    auto constexpr payloadD{ "D" };

    class TrainsTest : public testing::Test {
    protected:
        void SetUp() override {
            commLayer = make_unique<CommStub>();
            commLayerRaw = commLayer.get();
            algoLayer = make_unique<Trains>(std::move(commLayer));
            algoLayerRaw = algoLayer.get();
        }

        int nbSites;
        int nbTrains;
        vector<HostTuple> sites;
        unique_ptr<CommStub> commLayer;
        CommStub* commLayerRaw;
        unique_ptr<Trains> algoLayer;
        Trains* algoLayerRaw;
        rank_t myRank;
    };

    TEST_F(TrainsTest, ExecuteWith4SitesAndRank0And1Train) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 1;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();

        // Check established connection
        ASSERT_EQ(1, commLayerRaw->getConnectedDest().size());
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getConnectedDest()[0]);

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
        ASSERT_EQ(nbTrains, commLayerRaw->getSent().size());

        // Check that this message was sent to process 1
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);

        // Check contents of this message
        auto train{ deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second)) };
        ASSERT_EQ(0, train.id);
        ASSERT_EQ(1, train.clock);
        ASSERT_EQ(1, train.batches.size());

        fbae_AlgoLayer::BatchSessionMsg batch = train.batches[0];
        ASSERT_EQ(myRank, batch.senderPos);
        fbae_SessionLayer::SessionMsg message = batch.batchSessionMsg[0];
        ASSERT_EQ(SessionMsgId::FirstBroadcast, message->msgId);

        // Check if TotalOrderBroadcast is empty
        ASSERT_EQ(0, algoLayerRaw->getBatchWaitingSessionMsg().size());

        // Check @SessionLayer::callbackInitDone() was called
        ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

        // Check no message was delivered
        ASSERT_EQ(0, sessionStub.getDelivered().size());

        // Check clocks
        ASSERT_EQ(1, algoLayerRaw->getClock(0));
    }

    TEST_F(TrainsTest, ExecuteWith4SitesAndRank0And4Train) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 4;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();

        // Check established connection
        ASSERT_EQ(1, commLayerRaw->getConnectedDest().size());
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getConnectedDest()[0]);

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
        ASSERT_EQ(nbTrains, commLayerRaw->getSent().size());

        // Check that this message was sent to process 1
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);

        // Check contents of the first train
        auto trainInit{ deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second)) };
        ASSERT_EQ(0, trainInit.id);
        ASSERT_EQ(1, trainInit.clock);
        ASSERT_EQ(1, trainInit.batches.size());

        fbae_AlgoLayer::BatchSessionMsg batch = trainInit.batches[0];
        ASSERT_EQ(myRank, batch.senderPos);
        fbae_SessionLayer::SessionMsg message = batch.batchSessionMsg[0];
        ASSERT_EQ(SessionMsgId::FirstBroadcast, message->msgId);

        // Check contents of the other trains
        for (int i = 1; i < nbTrains; i++) {
            auto train{ deserializeStruct<Train>(std::move(commLayerRaw->getSent()[i].second)) };
            ASSERT_EQ(i, train.id);
            ASSERT_EQ(1, train.clock);
            ASSERT_EQ(0, train.batches.size());
        }

        // Check if TotalOrderBroadcast is empty
        ASSERT_EQ(0, algoLayerRaw->getBatchWaitingSessionMsg().size());

        // Check @SessionLayer::callbackInitDone() was called
        ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

        // Check no message was delivered
        ASSERT_EQ(0, sessionStub.getDelivered().size());

        // Check clocks
        ASSERT_EQ(1, algoLayerRaw->getClock(0));
        ASSERT_EQ(1, algoLayerRaw->getClock(1));
        ASSERT_EQ(1, algoLayerRaw->getClock(2));
        ASSERT_EQ(1, algoLayerRaw->getClock(3));
    }

    TEST_F(TrainsTest, ExecuteWith9SitesAndRank8And1Train) {
        nbSites = 9;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 8;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 1;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();

        // Check established connection
        ASSERT_EQ(1, commLayerRaw->getConnectedDest().size());
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getConnectedDest()[0]);

        // Check nbAwaitedConnections
        ASSERT_EQ(1, commLayerRaw->getNbAwaitedConnections());

        // Check @AlgoLayer:broadcastersGroup is correct
        ASSERT_EQ(nbSites, algoLayerRaw->getBroadcastersGroup().size());
        ASSERT_EQ(0, algoLayerRaw->getBroadcastersGroup()[0]);
        ASSERT_EQ(1, algoLayerRaw->getBroadcastersGroup()[1]);
        ASSERT_EQ(2, algoLayerRaw->getBroadcastersGroup()[2]);
        ASSERT_EQ(3, algoLayerRaw->getBroadcastersGroup()[3]);
        ASSERT_EQ(4, algoLayerRaw->getBroadcastersGroup()[4]);
        ASSERT_EQ(5, algoLayerRaw->getBroadcastersGroup()[5]);
        ASSERT_EQ(6, algoLayerRaw->getBroadcastersGroup()[6]);
        ASSERT_EQ(7, algoLayerRaw->getBroadcastersGroup()[7]);
        ASSERT_EQ(8, algoLayerRaw->getBroadcastersGroup()[8]);

        // Check Process is broadcasting messages
        ASSERT_TRUE(algoLayerRaw->isBroadcastingMessages());

        // Check no message was sent because rank is not 0
        ASSERT_EQ(0, commLayerRaw->getSent().size());

        // Check if TotalOrderBroadcast is empty
        ASSERT_EQ(1, algoLayerRaw->getBatchWaitingSessionMsg().size());

        // Check @SessionLayer::callbackInitDone() was called
        ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

        // Check no message was delivered
        ASSERT_EQ(0, sessionStub.getDelivered().size());

        // Check clocks
        ASSERT_EQ(0, algoLayerRaw->getClock(0));
    }

    static string buildTrain(int trainId) {
        return serializeStruct<Train>(Train{ trainId, 1, {} });
    }

    static string buildTrain(int trainId, vector<fbae_AlgoLayer::BatchSessionMsg> const& batches) {
        return serializeStruct<Train>(Train{ trainId, 1, batches });
    }

    static string buildTrain(int trainId, rank_t batchSender1, string const& payload1) {
        auto sessionMsg1 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload1);
        fbae_AlgoLayer::BatchSessionMsg batch{
                batchSender1,
                vector<SessionMsg>{sessionMsg1} };

        return buildTrain(trainId, vector<fbae_AlgoLayer::BatchSessionMsg>{ batch });
    }

    static string buildTrain(int trainId, rank_t batchSender1, string const& payload1, rank_t batchSender2, string const& payload2) {
        auto sessionMsg1 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload1);
        auto sessionMsg2 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload2);
        fbae_AlgoLayer::BatchSessionMsg batch1{
                batchSender1,
                vector<SessionMsg>{sessionMsg1} };
        fbae_AlgoLayer::BatchSessionMsg batch2{
                batchSender2,
                vector<SessionMsg>{sessionMsg2} };

        return buildTrain(trainId, vector<fbae_AlgoLayer::BatchSessionMsg>{ batch1, batch2 });
    }

    static fbae_AlgoLayer::BatchSessionMsg buildBatch(rank_t batchSender, string const& payload1) {
        auto sessionMsg1 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload1);
        return { batchSender, vector<SessionMsg>{sessionMsg1} };
    }

    static fbae_AlgoLayer::BatchSessionMsg buildBatch(rank_t batchSender, string const& payload1, string const& payload2) {
        auto sessionMsg1 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload1);
        auto sessionMsg2 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload2);
        return { batchSender, vector<SessionMsg>{sessionMsg1, sessionMsg2} };
    }

    TEST_F(TrainsTest, DeliverMessages) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 2;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests

        ASSERT_EQ(1, algoLayerRaw->getWaitingBatchesNb()); // Only the first broadcast is waiting for deliver

        // Waiting batches
        auto batch1 = buildBatch(myRank, payloadA);
        auto batch2 = buildBatch(static_cast<rank_t>((myRank + nbSites - 1) % nbSites), payloadB, payloadC);
        auto batch3 = buildBatch((myRank + 1) % nbSites, payloadD);

        algoLayerRaw->addWaitingBatch(0, batch1);
        algoLayerRaw->addWaitingBatch(0, batch2);
        algoLayerRaw->addWaitingBatch(1, batch3);

        ASSERT_EQ(4, algoLayerRaw->getWaitingBatchesNb());

        algoLayerRaw->callbackReceive(buildTrain(0, (myRank + 2) % nbSites, payloadD, (myRank + 1) % nbSites, payloadA));

        // Check all messages are delivered in the correct order
        ASSERT_EQ(4, sessionStub.getDelivered().size());

        EXPECT_EQ(myRank, sessionStub.getDelivered()[0].first);
        EXPECT_EQ(SessionMsgId::FirstBroadcast, sessionStub.getDelivered()[0].second->msgId);

        EXPECT_EQ(myRank, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(payloadA, sessionStub.getDelivered()[1].second->getPayload());

        EXPECT_EQ((myRank + nbSites - 1) % nbSites, sessionStub.getDelivered()[2].first);
        EXPECT_EQ(payloadB, sessionStub.getDelivered()[2].second->getPayload());

        EXPECT_EQ((myRank + nbSites - 1) % nbSites, sessionStub.getDelivered()[3].first);
        EXPECT_EQ(payloadC, sessionStub.getDelivered()[3].second->getPayload());

        // Check remainning waiting batches
        ASSERT_EQ(3, algoLayerRaw->getWaitingBatchesNb()); // 3 = 1 non delivered + 2 from train

        EXPECT_EQ((myRank + 1) % nbSites, algoLayerRaw->getPreviousTrainsBatches()[1][0].senderPos);
        EXPECT_EQ(payloadD, algoLayerRaw->getPreviousTrainsBatches()[1][0].batchSessionMsg[0]->getPayload());

        // Check algo clocks
        ASSERT_EQ(2, algoLayerRaw->getClock(0));
        ASSERT_EQ(1, algoLayerRaw->getClock(1));
    }

    TEST_F(TrainsTest, ReceiveBatches) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 2;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests

        algoLayerRaw->callbackReceive(buildTrain(0, (myRank + 2) % nbSites, payloadD, (myRank + 1) % nbSites, payloadA));

        // Check train messages are added to waiting messages
        ASSERT_EQ(2, algoLayerRaw->getWaitingBatchesNb());
        ASSERT_EQ(2, algoLayerRaw->getPreviousTrainsBatches()[0].size());
        ASSERT_EQ(0, algoLayerRaw->getPreviousTrainsBatches()[1].size());

        EXPECT_EQ((myRank + 2) % nbSites, algoLayerRaw->getPreviousTrainsBatches()[0][0].senderPos);
        EXPECT_EQ(payloadD, algoLayerRaw->getPreviousTrainsBatches()[0][0].batchSessionMsg[0]->getPayload());

        EXPECT_EQ((myRank + 1) % nbSites, algoLayerRaw->getPreviousTrainsBatches()[0][1].senderPos);
        EXPECT_EQ(payloadA, algoLayerRaw->getPreviousTrainsBatches()[0][1].batchSessionMsg[0]->getPayload());

        // Check algo clocks
        ASSERT_EQ(2, algoLayerRaw->getClock(0));
        ASSERT_EQ(1, algoLayerRaw->getClock(1));
    }

    TEST_F(TrainsTest, RemoveBatchFromTrain) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 2;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests

        algoLayerRaw->callbackReceive(buildTrain(0, (myRank + 2) % nbSites, payloadD, (myRank + 1) % nbSites, payloadA));

        // Check a new train has been sent
        ASSERT_EQ(1, commLayerRaw->getSent().size());
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);

        // Check train content
        auto train{ deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second)) };
        ASSERT_EQ(0, train.id);
        ASSERT_EQ(2, train.clock);
        ASSERT_EQ(1, train.batches.size());

        EXPECT_EQ((myRank + 2) % nbSites, train.batches[0].senderPos);
        EXPECT_EQ(payloadD, train.batches[0].batchSessionMsg[0]->getPayload());

        // Check algo clocks
        ASSERT_EQ(2, algoLayerRaw->getClock(0));
        ASSERT_EQ(1, algoLayerRaw->getClock(1));
    }

    TEST_F(TrainsTest, AddBatchToTrain) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 2;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests

        // Add messages to append to train
        auto sessionMsg1 = make_shared<SessionTest>(SessionMsgId::TestMessage, "A");
        algoLayerRaw->totalOrderBroadcast(sessionMsg1);
        auto sessionMsg2 = make_shared<SessionTest>(SessionMsgId::TestMessage, "C");
        algoLayerRaw->totalOrderBroadcast(sessionMsg2);

        algoLayerRaw->callbackReceive(buildTrain(0));

        // Check a new train has been sent
        ASSERT_EQ(1, commLayerRaw->getSent().size());
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);

        // Check train content
        auto train{ deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second)) };
        ASSERT_EQ(0, train.id);
        ASSERT_EQ(2, train.clock);
        ASSERT_EQ(1, train.batches.size());

        ASSERT_EQ(2, train.batches[0].batchSessionMsg.size());

        EXPECT_EQ(myRank, train.batches[0].senderPos);
        EXPECT_EQ(payloadA, train.batches[0].batchSessionMsg[0]->getPayload());
        EXPECT_EQ(payloadC, train.batches[0].batchSessionMsg[1]->getPayload());

        // Check algo clocks
        ASSERT_EQ(2, algoLayerRaw->getClock(0));
        ASSERT_EQ(1, algoLayerRaw->getClock(1));
    }

    TEST_F(TrainsTest, FullTrainProcess) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 2;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests

        ASSERT_EQ(1, algoLayerRaw->getWaitingBatchesNb()); // Only the first broadcast is waiting for deliver

        // Waiting batches
        auto batch1 = buildBatch(myRank, payloadA);
        auto batch2 = buildBatch(static_cast<rank_t>((myRank + nbSites - 1) % nbSites), payloadB, payloadC);
        auto batch3 = buildBatch((myRank + 1) % nbSites, payloadD);

        algoLayerRaw->addWaitingBatch(0, batch1);
        algoLayerRaw->addWaitingBatch(0, batch2);
        algoLayerRaw->addWaitingBatch(1, batch3);

        ASSERT_EQ(4, algoLayerRaw->getWaitingBatchesNb());

        // Add messages to append to train
        auto sessionMsg1 = make_shared<SessionTest>(SessionMsgId::TestMessage, "A");
        algoLayerRaw->totalOrderBroadcast(sessionMsg1);
        auto sessionMsg2 = make_shared<SessionTest>(SessionMsgId::TestMessage, "C");
        algoLayerRaw->totalOrderBroadcast(sessionMsg2);

        algoLayerRaw->callbackReceive(buildTrain(0, (myRank + 2) % nbSites, payloadD, (myRank + 1) % nbSites, payloadA));

        // Check algo clocks
        ASSERT_EQ(2, algoLayerRaw->getClock(0));
        ASSERT_EQ(1, algoLayerRaw->getClock(1));

        // Check all messages are delivered in the correct order
        ASSERT_EQ(4, sessionStub.getDelivered().size());

        EXPECT_EQ(myRank, sessionStub.getDelivered()[0].first);
        EXPECT_EQ(SessionMsgId::FirstBroadcast, sessionStub.getDelivered()[0].second->msgId);

        EXPECT_EQ(myRank, sessionStub.getDelivered()[1].first);
        EXPECT_EQ(payloadA, sessionStub.getDelivered()[1].second->getPayload());

        EXPECT_EQ((myRank + nbSites - 1) % nbSites, sessionStub.getDelivered()[2].first);
        EXPECT_EQ(payloadB, sessionStub.getDelivered()[2].second->getPayload());

        EXPECT_EQ((myRank + nbSites - 1) % nbSites, sessionStub.getDelivered()[3].first);
        EXPECT_EQ(payloadC, sessionStub.getDelivered()[3].second->getPayload());

        // Check waiting batches
        ASSERT_EQ(4, algoLayerRaw->getWaitingBatchesNb()); // 3 = 1 non delivered from train #1 + 2 from train #0 + 1 from outself
        ASSERT_EQ(3, algoLayerRaw->getPreviousTrainsBatches()[0].size());
        ASSERT_EQ(1, algoLayerRaw->getPreviousTrainsBatches()[1].size());

        // Check remainning waiting batches
        EXPECT_EQ((myRank + 1) % nbSites, algoLayerRaw->getPreviousTrainsBatches()[1][0].senderPos);
        EXPECT_EQ(payloadD, algoLayerRaw->getPreviousTrainsBatches()[1][0].batchSessionMsg[0]->getPayload());

        // Check train's batches are added to waiting messages
        EXPECT_EQ((myRank + 2) % nbSites, algoLayerRaw->getPreviousTrainsBatches()[0][0].senderPos);
        EXPECT_EQ(payloadD, algoLayerRaw->getPreviousTrainsBatches()[0][0].batchSessionMsg[0]->getPayload());

        EXPECT_EQ((myRank + 1) % nbSites, algoLayerRaw->getPreviousTrainsBatches()[0][1].senderPos);
        EXPECT_EQ(payloadA, algoLayerRaw->getPreviousTrainsBatches()[0][1].batchSessionMsg[0]->getPayload());

        // Check our own message
        EXPECT_EQ(myRank, algoLayerRaw->getPreviousTrainsBatches()[0][2].senderPos);
        ASSERT_EQ(2, algoLayerRaw->getPreviousTrainsBatches()[0][2].batchSessionMsg.size());
        EXPECT_EQ(payloadA, algoLayerRaw->getPreviousTrainsBatches()[0][2].batchSessionMsg[0]->getPayload());
        EXPECT_EQ(payloadC, algoLayerRaw->getPreviousTrainsBatches()[0][2].batchSessionMsg[1]->getPayload());

        // Check a new train has been sent
        ASSERT_EQ(1, commLayerRaw->getSent().size());
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);

        // Check train content
        auto train{ deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second)) };
        ASSERT_EQ(0, train.id);
        ASSERT_EQ(2, train.clock);
        ASSERT_EQ(2, train.batches.size());

        ASSERT_EQ(1, train.batches[0].batchSessionMsg.size());
        ASSERT_EQ(2, train.batches[1].batchSessionMsg.size());

        EXPECT_EQ((myRank + 2) % nbSites, train.batches[0].senderPos);
        EXPECT_EQ(payloadD, train.batches[0].batchSessionMsg[0]->getPayload());

        EXPECT_EQ(myRank, train.batches[1].senderPos);
        EXPECT_EQ(payloadA, train.batches[1].batchSessionMsg[0]->getPayload());
        EXPECT_EQ(payloadC, train.batches[1].batchSessionMsg[1]->getPayload());
    }

    TEST_F(TrainsTest, LateTrain) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 2;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests

        algoLayerRaw->callbackReceive(buildTrain(0));
        commLayerRaw->getSent().clear(); // We clear the first train information which we already tested in previous execute() tests
        ASSERT_DEATH(algoLayerRaw->callbackReceive(buildTrain(0)), "Lost train #0 with clock: 1. Intern clock: 2");
    }

    TEST_F(TrainsTest, UnrecognizeTrain) {
        nbSites = 4;
        sites = vector<HostTuple>(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };
        nbTrains = 2;
        algoLayerRaw->setNbTrains(nbTrains);

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
        
        ASSERT_DEATH(algoLayerRaw->callbackReceive(buildTrain(2)), "Unexpected Train with id 2. Max trains id: 1");
    }
}