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

    TEST(Sequencer, SequencerConnections) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto comm = commLayer.get();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algo = algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank, std::move(algoLayer)};

        algo->execute();

        EXPECT_EQ(nbSites - 1, comm->getConnectedDest().size());
        EXPECT_EQ(1, comm->getConnectedDest()[0]);
        EXPECT_EQ(2, comm->getConnectedDest()[1]);
        EXPECT_EQ(3, comm->getConnectedDest()[2]);
    }

    TEST(Sequencer, BroadcasterConnections) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto comm = commLayer.get();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algo = algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank + 1, std::move(algoLayer)};

        algo->execute();

        EXPECT_EQ(1, comm->getConnectedDest().size());
        EXPECT_EQ(sequencerRank, comm->getConnectedDest()[0]);
    }

    TEST(Sequencer, TotalOrderBroadcast) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto comm = commLayer.get();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algo = algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank + 1, std::move(algoLayer)};

        algo->execute();
        constexpr auto payload{"A"};
        auto s {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                                    payload})};
        algo->totalOrderBroadcast(std::move(s));

        // Check that 1 message was sent
        EXPECT_EQ(1, comm->getSent().size());
        // Check that this message was sent to sequencer
        EXPECT_EQ(sequencerRank, comm->getSent()[0].first);
        // Check contents of this message
        auto msgBroadcast{deserializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(std::move(comm->getSent()[0].second))};
        EXPECT_EQ(fbae_SequencerAlgoLayer::MsgId::BroadcastRequest, msgBroadcast.msgId);
        EXPECT_EQ(0, msgBroadcast.senderPos); // 0 because broadcaster with rank 1 has position 0 among broadcasters
        auto sessionMsg{deserializeStruct<SessionTest>(std::move(msgBroadcast.sessionMsg))};
        EXPECT_EQ(SessionMsgId::TestMessage, sessionMsg.msgId);
        EXPECT_EQ(payload, sessionMsg.payload);
    }

    TEST(Sequencer, SequencerReceivesBroadcastRequest) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto comm = commLayer.get();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algo = algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank, std::move(algoLayer)};

        algo->execute();
        constexpr auto payload{"A"};
        constexpr rank_t senderPos = 42;
        auto s {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                         payload})};
        auto msg {serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(fbae_SequencerAlgoLayer::StructBroadcastMessage{fbae_SequencerAlgoLayer::MsgId::BroadcastRequest,
                                                                               senderPos,
                                                                               std::move(s)})};
        algo->callbackHandleMessage(std::move(msg));

        // Check that 3 messages are sent by Sequencer
        EXPECT_EQ(nbSites-1, comm->getSent().size());
        // Check that destinations of all of these messages
        EXPECT_EQ(1, comm->getSent()[0].first);
        EXPECT_EQ(2, comm->getSent()[1].first);
        EXPECT_EQ(3, comm->getSent()[2].first);
        // Check second and third messages are equal to first message.
        EXPECT_EQ(comm->getSent()[0].second, comm->getSent()[1].second);
        EXPECT_EQ(comm->getSent()[0].second, comm->getSent()[2].second);
        // Check contents of the first message
        auto msgBroadcast{deserializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(std::move(comm->getSent()[0].second))};
        EXPECT_EQ(fbae_SequencerAlgoLayer::MsgId::Broadcast, msgBroadcast.msgId);
        EXPECT_EQ(senderPos, msgBroadcast.senderPos);
        auto sessionMsg{deserializeStruct<SessionTest>(std::move(msgBroadcast.sessionMsg))};
        EXPECT_EQ(SessionMsgId::TestMessage, sessionMsg.msgId);
        EXPECT_EQ(payload, sessionMsg.payload);
    }

    TEST(Sequencer, ParticipantReceivesBroadcast) {
        constexpr auto nbSites = 4;
        vector<HostTuple> sites(nbSites, HostTuple{"", 0});
        Arguments arguments{sites};
        auto commLayer = make_unique<CommStub>();
        auto algoLayer = make_unique<Sequencer>(std::move(commLayer));
        auto algo = algoLayer.get();
        SessionStub sessionStub{arguments, sequencerRank + 1, std::move(algoLayer)};

        algo->execute();
        constexpr auto payload{"A"};
        constexpr rank_t senderPos = 42;
        auto s {serializeStruct<SessionTest>(SessionTest{SessionMsgId::TestMessage,
                                                         payload})};
        auto msg {serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(fbae_SequencerAlgoLayer::StructBroadcastMessage{fbae_SequencerAlgoLayer::MsgId::Broadcast,
                                                                                                                                   senderPos,
                                                                                                                                   std::move(s)})};
        algo->callbackHandleMessage(std::move(msg));

        // Check that 1 message has been delivered
        EXPECT_EQ(1, sessionStub.getDelivered().size());
        // Check sender of this message
        EXPECT_EQ(senderPos, sessionStub.getDelivered()[0].first);
        // Check contents of this message
        auto sessionMsg{deserializeStruct<SessionTest>(std::move(sessionStub.getDelivered()[0].second))};
        EXPECT_EQ(SessionMsgId::TestMessage, sessionMsg.msgId);
        EXPECT_EQ(payload, sessionMsg.payload);
    }
}