#include <gtest/gtest.h>

#include "msgTemplates.h"
#include "AlgoLayer/Trains/Trains.h"

#include "../../../../tests/CommLayer/CommStub.h"
#include "../../../../tests/SessionLayer/SessionStub.h"

namespace fbae_test_LCR {

    using namespace std;
    using namespace fbae_SessionLayer;

    TEST(Trains, TrainsExecute) {
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
        ASSERT_EQ(1, commLayerRaw->getConnectedDest()[0]);

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
        ASSERT_EQ(1, commLayerRaw->getSent()[0].first);
        // Check contents of this message
        auto broadcastMsg{deserializeStruct<Train>(std::move(commLayerRaw->getSent()[0].second))};
        ASSERT_EQ(1, broadcastMsg.clock);
        for (const auto &wagon: broadcastMsg.wagons) {
            ASSERT_EQ(myRank, wagon.sender);
            for (const auto &message: wagon.msgs) {
                ASSERT_EQ(SessionMsgId::FirstBroadcast, message.message->msgId);
            }
        }

        // Check plain Participant is broadcasting messages
//        ASSERT_TRUE(algoLayerRaw->isBroadcastingMessages());

        // Check @SessionLayer::callbackInitDone() was called
        ASSERT_TRUE(sessionStub.isCallbackInitDoneCalled());

        // Check no message was delivered
        ASSERT_EQ(0, sessionStub.getDelivered().size());
    }
}