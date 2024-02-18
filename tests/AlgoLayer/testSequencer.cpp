//
// Created by simatic on 2/17/24.
//

#include <gtest/gtest.h>
#include "../../src/msgTemplates.h"
#include "../../src/AlgoLayer/Sequencer/Sequencer.h"
#include "../../src/AlgoLayer/Sequencer/SequencerMsg.h"
#include "../../src/SessionLayer/SessionLayerMsg.h"
#include "../CommLayer/CommStub.h"
#include "../SessionLayer/SessionStub.h"

namespace fbae_test_Sequencer {

    using namespace std;
    using namespace fbae_SessionLayer;

    TEST(Sequencer, SequencerExecute) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank, std::move(algoLayer)};

        algoLayerRaw->execute();

        // Check established connections
        EXPECT_EQ(nbSites - 1, commLayerRaw->getConnectedDest().size());
        EXPECT_EQ(1, commLayerRaw->getConnectedDest()[0]);
        EXPECT_EQ(2, commLayerRaw->getConnectedDest()[1]);
        EXPECT_EQ(3, commLayerRaw->getConnectedDest()[2]);

        // Check nbAwaitedConnections
        EXPECT_EQ(nbSites - 1, commLayerRaw->getNbAwaitedConnections());

        // Check no message was sent
        EXPECT_EQ(0, commLayerRaw->getSent().size());

        // Check @AlgoLayer:broadcastersGroup is correct
        EXPECT_EQ(nbSites - 1, algoLayerRaw->getBroadcastersGroup().size());
        EXPECT_EQ(1, algoLayerRaw->getBroadcastersGroup()[0]);
        EXPECT_EQ(2, algoLayerRaw->getBroadcastersGroup()[1]);
        EXPECT_EQ(3, algoLayerRaw->getBroadcastersGroup()[2]);

        // Check Sequencer is not broadcasting messages
        EXPECT_FALSE(algoLayerRaw->isBroadcastingMessages());

        // Check @SessionLayer::callbackInitDone() was called
        EXPECT_TRUE(sessionStub.isCallbackInitdoneCalled());

        // Check no message was delivered
        EXPECT_EQ(0, sessionStub.getDelivered().size());
    }

    TEST(Sequencer, BroadcasterExecute) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank + 1, std::move(algoLayer)};

        algoLayerRaw->execute();

        // Check established connections
        EXPECT_EQ(1, commLayerRaw->getConnectedDest().size());
        EXPECT_EQ(sequencerRank, commLayerRaw->getConnectedDest()[0]);

        // Check nbAwaitedConnections
        EXPECT_EQ(1, commLayerRaw->getNbAwaitedConnections());

        // Check no message was sent
        EXPECT_EQ(0, commLayerRaw->getSent().size());

        // Check @AlgoLayer:broadcastersGroup is correct
        EXPECT_EQ(nbSites - 1, algoLayerRaw->getBroadcastersGroup().size());
        EXPECT_EQ(1, algoLayerRaw->getBroadcastersGroup()[0]);
        EXPECT_EQ(2, algoLayerRaw->getBroadcastersGroup()[1]);
        EXPECT_EQ(3, algoLayerRaw->getBroadcastersGroup()[2]);

        // Check Broadcaster is broadcasting messages
        EXPECT_TRUE(algoLayerRaw->isBroadcastingMessages());

        // Check @SessionLayer::callbackInitDone() was called
        EXPECT_TRUE(sessionStub.isCallbackInitdoneCalled());

        // Check no message was delivered
        EXPECT_EQ(0, sessionStub.getDelivered().size());
    }

    TEST(Sequencer, TotalOrderBroadcast) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank + 1, std::move(algoLayer)};

        algoLayerRaw->execute();
        constexpr auto payload{"A"};
        auto s {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                    payload})};
        auto sCopy{s};
        algoLayerRaw->totalOrderBroadcast(std::move(s));

        // Check that 1 message was sent
        EXPECT_EQ(1, commLayerRaw->getSent().size());
        // Check that this message was sent to sequencer
        EXPECT_EQ(sequencerRank, commLayerRaw->getSent()[0].first);
        // Check contents of this message
        auto msgBroadcast{deserializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_SequencerAlgoLayer::MsgId::BroadcastRequest, msgBroadcast.msgId);
        EXPECT_EQ(0, msgBroadcast.senderPos); // 0 because broadcaster with rank 1 has position 0 among broadcasters
        EXPECT_EQ(sCopy, msgBroadcast.sessionMsg);
    }

    TEST(Sequencer, SequencerReceivesBroadcastRequest) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto commLayerRaw= commLayer.get();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank, std::move(algoLayer)};

        algoLayerRaw->execute();
        constexpr auto payload{"A"};
        constexpr rank_t senderPos = 42;
        auto s {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                         payload})};
        auto sCopy{s};
        auto msg {serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(fbae_SequencerAlgoLayer::StructBroadcastMessage{fbae_SequencerAlgoLayer::MsgId::BroadcastRequest,
                                                                               senderPos,
                                                                               std::move(s)})};
        algoLayerRaw->callbackReceive(std::move(msg));

        // Check that 3 messages are sent by Sequencer
        EXPECT_EQ(nbSites-1, commLayerRaw->getSent().size());
        // Check that destinations of all of these messages
        EXPECT_EQ(1, commLayerRaw->getSent()[0].first);
        EXPECT_EQ(2, commLayerRaw->getSent()[1].first);
        EXPECT_EQ(3, commLayerRaw->getSent()[2].first);
        // Check second and third messages are equal to first message.
        EXPECT_EQ(commLayerRaw->getSent()[0].second, commLayerRaw->getSent()[1].second);
        EXPECT_EQ(commLayerRaw->getSent()[0].second, commLayerRaw->getSent()[2].second);
        // Check contents of the first message
        auto msgBroadcast{deserializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(std::move(commLayerRaw->getSent()[0].second))};
        EXPECT_EQ(fbae_SequencerAlgoLayer::MsgId::Broadcast, msgBroadcast.msgId);
        EXPECT_EQ(senderPos, msgBroadcast.senderPos);
        EXPECT_EQ(sCopy, msgBroadcast.sessionMsg);
    }

    TEST(Sequencer, ParticipantReceivesBroadcast) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algoLayerRaw= algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank + 1, std::move(algoLayer)};

        algoLayerRaw->execute();
        constexpr auto payload{"A"};
        constexpr rank_t senderPos = 42;
        auto s {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                         payload})};
        auto sCopy{s};
        auto msg {serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(fbae_SequencerAlgoLayer::StructBroadcastMessage{fbae_SequencerAlgoLayer::MsgId::Broadcast,
                                                                                                                                   senderPos,
                                                                                                                                   std::move(s)})};
        algoLayerRaw->callbackReceive(std::move(msg));

        // Check that 1 message has been delivered
        EXPECT_EQ(1, sessionStub.getDelivered().size());
        // Check sender of this message
        EXPECT_EQ(senderPos, sessionStub.getDelivered()[0].first);
        // Check contents of this message
        EXPECT_EQ(sCopy, sessionStub.getDelivered()[0].second);
    }
}