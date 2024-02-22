//
// Created by simatic on 2/5/24.
//
#include <gtest/gtest.h>
#include <vector>
#include "../src/Arguments.h"
#include "../src/msgTemplates.h"
#include "../src/SessionLayer/SessionLayerMsg.h"
#include "../src/AlgoLayer/Sequencer/SequencerMsg.h"
#include "../src/AlgoLayer/BBOBB/BBOBBMsg.h"

namespace fbae_test_serializationOverhead {

    using namespace std;

    constexpr size_t sizeStringInSessionMsg = 9;

    TEST(SerializationOverhead, SerializedSessionMsg) {
        auto s_sessionMsg {serializeStruct<fbae_SessionLayer::SessionTest>(fbae_SessionLayer::SessionTest{fbae_SessionLayer::SessionMsgId::TestMessage,
                                                                                                          string(
                                                                                                                    sizeStringInSessionMsg,
                                                                                                                    'A')})};
        // Serialization overhead is sizeof(CEREAL_SIZE_TYPE) because Cereal need to store the size of the string.
        EXPECT_EQ(sizeof(CEREAL_SIZE_TYPE) + sizeof(fbae_SessionLayer::SessionMsgId)+sizeStringInSessionMsg,
                  s_sessionMsg.size());
    }

    TEST(SerializationOverhead, SerializedSequencerMsg) {
        auto s_sessionMsg {serializeStruct<fbae_SessionLayer::SessionTest>(fbae_SessionLayer::SessionTest{fbae_SessionLayer::SessionMsgId::TestMessage,
                                                                                                          string(
                                                                                                              sizeStringInSessionMsg,
                                                                                                              'A')})};
        auto s_StructBroadcastMessage {serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(fbae_SequencerAlgoLayer::StructBroadcastMessage{fbae_SequencerAlgoLayer::MsgId::Broadcast,
                                                                                                                                                        '1',
                                                                                                                                                        s_sessionMsg})};
        // Serialization overhead is sizeof(fbae_SequencerAlgoLayer::MsgId)+sizeof(senderPos)+sizeof(CEREAL_SIZE_TYPE) (because Cereal need to store the size of the string) + overhead on session message.
        EXPECT_EQ(sizeof(fbae_SequencerAlgoLayer::MsgId) + sizeof(rank_t) + sizeof(CEREAL_SIZE_TYPE) + (sizeof(CEREAL_SIZE_TYPE) + sizeof(fbae_SessionLayer::SessionMsgId)+sizeStringInSessionMsg),
                  s_StructBroadcastMessage.size());
    }

    TEST(SerializationOverhead, SerializedBatchMsg) {
        auto s_sessionMsg {serializeStruct<fbae_SessionLayer::SessionTest>(fbae_SessionLayer::SessionTest{fbae_SessionLayer::SessionMsgId::TestMessage,
                                                                                                          string(
                                                                                                                                                    sizeStringInSessionMsg,
                                                                                                                                                    'A')})};
        constexpr auto nbSessionMsgPerBatch = 1;
        std::vector<std::string> v_11{s_sessionMsg};
        fbae_AlgoLayer::BatchSessionMsg batchSessionMsg_11 {
                '1',
                v_11};
        auto s_batchSessionMsg_11 {serializeStruct<fbae_AlgoLayer::BatchSessionMsg>(batchSessionMsg_11)};

        constexpr auto sizeHeaderAndSizeVectorEncodingInBatchSessionMsg = sizeof(fbae_BBOBBAlgoLayer::MsgId) + sizeof(CEREAL_SIZE_TYPE); // sizeof(CEREAL_SIZE_TYPE) for the encoding of the size of the vector
        EXPECT_EQ(sizeHeaderAndSizeVectorEncodingInBatchSessionMsg + nbSessionMsgPerBatch * (sizeof(CEREAL_SIZE_TYPE) + s_sessionMsg.size()),
                  s_batchSessionMsg_11.size());
    }

    TEST(SerializationOverhead, SerializedBBOBBStepMsg) {
        auto s_sessionMsg {serializeStruct<fbae_SessionLayer::SessionTest>(fbae_SessionLayer::SessionTest{fbae_SessionLayer::SessionMsgId::TestMessage,
                                                                                                          string(
                                                                                                                                                    sizeStringInSessionMsg,
                                                                                                                                                    'A')})};
        constexpr auto nbSessionMsgPerBatch = 1;
        std::vector<std::string> v_11{s_sessionMsg};
        fbae_AlgoLayer::BatchSessionMsg batchSessionMsg_11 {
                '1',
                v_11};
        constexpr auto sizeHeaderAndSizeVectorEncodingInBatchSessionMsg = sizeof(fbae_BBOBBAlgoLayer::MsgId) + sizeof(CEREAL_SIZE_TYPE); // sizeof(CEREAL_SIZE_TYPE) for the encoding of the size of the vector

        constexpr auto nbBatchInStep = 1;
        std::vector<fbae_AlgoLayer::BatchSessionMsg> v_batchSessionMsg_11{batchSessionMsg_11};
        auto s_Step_11 {serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step,
                                                                                                   '3',
                                                                                                   42,
                                                                                                   55,
                                                                                                   v_batchSessionMsg_11})};
        constexpr auto sizeHeaderAndSizeVectorEncodingInStepMsg = sizeof(fbae_BBOBBAlgoLayer::MsgId) + sizeof(rank_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(CEREAL_SIZE_TYPE); // sizeof(CEREAL_SIZE_TYPE) for the encoding of the size of the vector
        EXPECT_EQ(sizeHeaderAndSizeVectorEncodingInStepMsg + nbBatchInStep * (sizeHeaderAndSizeVectorEncodingInBatchSessionMsg + nbSessionMsgPerBatch *(sizeof(CEREAL_SIZE_TYPE) + s_sessionMsg.size())),
                  s_Step_11.size());
    }

    TEST(SerializationOverhead, CheckMinSizeClientMessageToBroadcast) {
        auto s_sessionMsg {serializeStruct<fbae_SessionLayer::SessionPerfMeasure>(fbae_SessionLayer::SessionPerfMeasure{fbae_SessionLayer::SessionMsgId::PerfMeasure,
                                                                                                                        0,
                                                                                                                        0,
                                                                                                                        std::chrono::system_clock::now(),
                                                                                                                        ""})};
        // Serialization overhead is sizeof(CEREAL_SIZE_TYPE) because Cereal need to store the size of the string.
        EXPECT_EQ(minSizeClientMessageToBroadcast,
                  s_sessionMsg.size());
    }
}
