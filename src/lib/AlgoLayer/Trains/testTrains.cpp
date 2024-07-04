#include <gtest/gtest.h>

#include "msgTemplates.h"
#include "AlgoLayer/Trains/Trains.h"

#include "../../../../tests/CommLayer/CommStub.h"
#include "../../../../tests/SessionLayer/SessionStub.h"

namespace fbae_test_LCR {

    using namespace std;
    using namespace fbae_SessionLayer;

    auto constexpr payloadA{ "A" };
    auto constexpr payloadB{ "B" };

    TEST(Trains, ExecuteWith4SitesAndRank0) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw = commLayer.get();
        auto algoLayer = make_unique<Trains>(std::move(commLayer));
        auto algoLayerRaw = algoLayer.get();
        rank_t myRank = 0;
        SessionStub sessionStub{arguments, myRank, std::move(algoLayer)};

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
        ASSERT_EQ(1, commLayerRaw->getSent().size());

        // Check that this message was sent to process 1
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);

        // Check contents of this message
        auto broadcastMsg{deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second))};
        ASSERT_EQ(0, broadcastMsg.id);
        ASSERT_EQ(1, broadcastMsg.clock);

        for (const auto &wagon: broadcastMsg.wagons) {
            ASSERT_EQ(myRank, wagon.batch.senderPos);
            ASSERT_EQ(0, wagon.trainId);

            for (const auto &message: wagon.batch.batchSessionMsg) {
                ASSERT_EQ(SessionMsgId::FirstBroadcast, message->msgId);
            }
        }

        // Check @SessionLayer::callbackInitDone() was called
        ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

        // Check no message was delivered
        ASSERT_EQ(0, sessionStub.getDelivered().size());
    }

    TEST(Trains, ExecuteWith9SitesAndRank8) {
        constexpr auto nbSites = 9;
        vector<HostTuple> sites(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw = commLayer.get();
        auto algoLayer = make_unique<Trains>(std::move(commLayer));
        auto algoLayerRaw = algoLayer.get();
        rank_t myRank = 8;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };

        algoLayerRaw->execute();

        // Check established connection
        ASSERT_EQ(1, commLayerRaw->getConnectedDest().size());
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getConnectedDest()[0]);

        // Check nbAwaitedConnections
        ASSERT_EQ(1, commLayerRaw->getNbAwaitedConnections());

        // Check @AlgoLayer:broadcastersGroup is correct
        ASSERT_EQ(nbSites, algoLayerRaw->getBroadcastersGroup().size());
        EXPECT_EQ(0, algoLayerRaw->getBroadcastersGroup()[0]);
        EXPECT_EQ(1, algoLayerRaw->getBroadcastersGroup()[1]);
        EXPECT_EQ(2, algoLayerRaw->getBroadcastersGroup()[2]);
        EXPECT_EQ(3, algoLayerRaw->getBroadcastersGroup()[3]);
        EXPECT_EQ(4, algoLayerRaw->getBroadcastersGroup()[4]);
        EXPECT_EQ(5, algoLayerRaw->getBroadcastersGroup()[5]);
        EXPECT_EQ(6, algoLayerRaw->getBroadcastersGroup()[6]);
        EXPECT_EQ(7, algoLayerRaw->getBroadcastersGroup()[7]);
        EXPECT_EQ(8, algoLayerRaw->getBroadcastersGroup()[8]);

        // Check Process is broadcasting messages
        ASSERT_TRUE(algoLayerRaw->isBroadcastingMessages());

        // Check FirstBroadcast message was broadcast and thus sent.
        ASSERT_EQ(1, commLayerRaw->getSent().size());

        // Check that this message was sent to process 1
        ASSERT_EQ((myRank + 1) % nbSites, commLayerRaw->getSent()[0].first);

        // Check contents of this message
        auto broadcastMsg{ deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second)) };
        ASSERT_EQ(0, broadcastMsg.id);
        ASSERT_EQ(1, broadcastMsg.clock);

        for (const auto& wagon : broadcastMsg.wagons) {
            ASSERT_EQ(myRank, wagon.batch.senderPos);
            ASSERT_EQ(0, wagon.trainId);

            for (const auto& message : wagon.batch.batchSessionMsg) {
                ASSERT_EQ(SessionMsgId::FirstBroadcast, message->msgId);
            }
        }

        // Check @SessionLayer::callbackInitDone() was called
        ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

        // Check no message was delivered
        ASSERT_EQ(0, sessionStub.getDelivered().size());
    }

    static string buildTrain(int trainId, vector<Wagon> const& wagons) {
        return serializeStruct<Train>(Train{ trainId, 0, wagons });
    }

    static string buildTrain(int trainId, rank_t batchSender, string const& payload1) {
        auto sessionMsg1 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload1);
        fbae_AlgoLayer::BatchSessionMsg batch{
                batchSender,
                vector<SessionMsg>{sessionMsg1} };

        return buildTrain(trainId, vector<Wagon>{ {trainId, batch} });
    }

    static string buildTrain(int trainId, rank_t batchSender, string const& payload1, string const& payload2) {
        auto sessionMsg1 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload1);
        auto sessionMsg2 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload2);
        fbae_AlgoLayer::BatchSessionMsg batch1{
                batchSender,
                vector<SessionMsg>{sessionMsg1} };
        fbae_AlgoLayer::BatchSessionMsg batch2{
                batchSender,
                vector<SessionMsg>{sessionMsg2} };

        return buildTrain(trainId, vector<Wagon>{ {trainId, batch1}, { trainId, batch2 } });
    }

    static string buildWagon(int trainId, fbae_AlgoLayer::BatchSessionMsg const& b) {
        return serializeStruct<Wagon>(Wagon{ trainId, b });
    }

    static string buildWagon(int trainId, rank_t batchSender, string const& payload1) {
        auto sessionMsg1 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload1);
        fbae_AlgoLayer::BatchSessionMsg batch{
                batchSender,
                vector<SessionMsg>{sessionMsg1} };

        return buildWagon(trainId, batch);
    }

    static string buildWagon(int trainId, rank_t batchSender, string const& payload1, string const& payload2) {
        auto sessionMsg1 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload1);
        auto sessionMsg2 = make_shared<SessionTest>(
            SessionMsgId::TestMessage,
            payload2);
        fbae_AlgoLayer::BatchSessionMsg batch{
                batchSender,
                vector<SessionMsg>{sessionMsg1, sessionMsg2} };

        return buildWagon(trainId, batch);
    }


    TEST(Trains, ExecuteWith4SitesAndRank0ReceiveTrain) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{ "", 0 });
        Arguments arguments{ sites };
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw = commLayer.get();
        auto algoLayer = make_unique<Trains>(std::move(commLayer));
        auto algoLayerRaw = algoLayer.get();
        rank_t myRank = 0;
        SessionStub sessionStub{ arguments, myRank, std::move(algoLayer) };

        algoLayerRaw->execute();
        commLayerRaw->getSent().clear(); // We clear FirstBroadcast information which we already tested in previous execute() tests
        // Prepare Step message to be received in current wave from sender 0
        algoLayerRaw->callbackReceive(buildTrain(0, myRank, payloadA, payloadB));







        // Check contents of this message
        auto broadcastMsg{ deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second)) };
        ASSERT_EQ(0, broadcastMsg.id);
        ASSERT_EQ(1, broadcastMsg.clock);

        for (const auto& wagon : broadcastMsg.wagons) {
            ASSERT_EQ(myRank, wagon.batch.senderPos);
            ASSERT_EQ(0, wagon.trainId);

            for (const auto& message : wagon.batch.batchSessionMsg) {
                ASSERT_EQ(SessionMsgId::FirstBroadcast, message->msgId);
            }
        }

        // Check @SessionLayer::callbackInitDone() was called
        ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

        // Check no message was delivered
        ASSERT_EQ(0, sessionStub.getDelivered().size());
    }

}