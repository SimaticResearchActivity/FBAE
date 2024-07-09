/**

    @file      testSerializationOverhead.cpp
    @brief     Unitary tests related to serialization overhead
    @details
    @author    Michel Simatic
    @date      2/5/24
    @copyright GNU Affero General Public License

**/
#include <gtest/gtest.h>

#include <vector>

#include "AlgoLayer/BBOBB/BBOBBMsg.h"
#include "AlgoLayer/Sequencer/SequencerMsg.h"
#include "Arguments.h"
#include "SessionLayer/SessionLayerMsg.h"
#include "msgTemplates.h"

namespace fbae_test_serializationOverhead {

using namespace std;

constexpr size_t sizeStringInSessionMsg = 9;

constexpr size_t overheadCerealPolymorphicEncoding =
    13;  // This value was determined experimentally

// sizeOfEncodedSessionMsg is overheadCerealPolymorphicEncoding
//                             + sizeof(fbaeSL::SessionMsgId)
//                             + sizeof(CEREAL_SIZE_TYPE) (because Cereal need
//                             to store the size of the string)
//                             + sizeStringInSessionMsg
constexpr size_t sizeOfEncodedSessionMsg =
    overheadCerealPolymorphicEncoding +
    sizeof(fbae_SessionLayer::SessionMsgId) + sizeof(CEREAL_SIZE_TYPE) +
    sizeStringInSessionMsg;

TEST(SerializationOverheadTest, SerializedSequencerMsg) {
  auto sessionMsg = make_shared<fbae_SessionLayer::SessionTest>(
      fbae_SessionLayer::SessionMsgId::TestMessage,
      string(sizeStringInSessionMsg, 'A'));
  auto s_StructBroadcastMessage{
      serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(
          fbae_SequencerAlgoLayer::StructBroadcastMessage{
              fbae_SequencerAlgoLayer::MsgId::Broadcast, '1', sessionMsg})};
  // Serialization overhead is
  // sizeof(fbae_SequencerAlgoLayer::MsgId)+sizeof(senderPos).
  EXPECT_EQ(sizeof(fbae_SequencerAlgoLayer::MsgId) + sizeof(rank_t) +
                sizeOfEncodedSessionMsg,
            s_StructBroadcastMessage.size());
}

TEST(SerializationOverhead, SerializedBBOBBStepMsg) {
  auto sessionMsg = make_shared<fbae_SessionLayer::SessionTest>(
      fbae_SessionLayer::SessionMsgId::TestMessage,
      string(sizeStringInSessionMsg, 'A'));
  constexpr auto nbSessionMsgPerBatch = 1;
  std::vector<std::shared_ptr<fbae_SessionLayer::SessionBaseClass>> v_11{
      sessionMsg};
  fbae_AlgoLayer::BatchSessionMsg batchSessionMsg_11{'1', v_11};

  constexpr auto nbBatchInStep = 1;
  std::vector<fbae_AlgoLayer::BatchSessionMsg> v_batchSessionMsg_11{
      batchSessionMsg_11};
  auto s_Step_11{serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(
      fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step, '3', 42,
                                   55, v_batchSessionMsg_11})};
  constexpr auto sizeHeaderAndSizeVectorEncodingInStepMsg =
      sizeof(fbae_BBOBBAlgoLayer::MsgId) + sizeof(rank_t) + sizeof(uint8_t) +
      sizeof(uint8_t) +
      sizeof(CEREAL_SIZE_TYPE);  // sizeof(CEREAL_SIZE_TYPE) for the encoding of
                                 // the size of the vector
  EXPECT_EQ(
      sizeHeaderAndSizeVectorEncodingInStepMsg +
          nbBatchInStep * (sizeof(fbae_AlgoLayer::BatchSessionMsg::senderPos) +
                           sizeof(CEREAL_SIZE_TYPE) +
                           nbSessionMsgPerBatch * sizeOfEncodedSessionMsg),
      s_Step_11.size());
}

TEST(SerializationOverhead, CheckMinSizeClientMessageToBroadcast) {
  auto sessionPerf = make_shared<fbae_SessionLayer::SessionPerf>(
      fbae_SessionLayer::SessionMsgId::PerfMeasure, 0, 0,
      std::chrono::system_clock::now(), "");
  // We store this session message in a fbae_SequencerAlgoLayer::MessagePacket
  // so that it is serialized
  auto s{serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(
      fbae_SequencerAlgoLayer::StructBroadcastMessage{
          fbae_SequencerAlgoLayer::MsgId::Broadcast, '1', sessionPerf})};

  // We compare minSizeClientMessageToBroadcast with the serialization (without
  // header of fbae_SequencerAlgoLayer::MessagePacket)
  EXPECT_EQ(minSizeClientMessageToBroadcast,
            s.size() - sizeof(fbae_SequencerAlgoLayer::MsgId) - sizeof(rank_t));
}
}  // namespace fbae_test_serializationOverhead
