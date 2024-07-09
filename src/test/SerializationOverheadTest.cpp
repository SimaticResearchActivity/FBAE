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
#include "msgTemplates.h"

namespace fbae::core {

using namespace std;
using namespace fbae::core;
using namespace fbae::core::AlgoLayer::Sequencer;
using namespace fbae::core::AlgoLayer::BBOBB;
using namespace fbae::core::SessionLayer;
using fbae::core::AlgoLayer::BatchSessionMsg;

using enum fbae::core::SessionLayer::SessionMsgId;

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
    sizeof(SessionMsgId) + sizeof(CEREAL_SIZE_TYPE) +
    sizeStringInSessionMsg;

TEST(SerializationOverheadTest, SerializedSequencerMsg) {
  auto sessionMsg = make_shared<SessionTest>(
      TestMessage,
      string(sizeStringInSessionMsg, 'A'));
  auto s_StructBroadcastMessage{
      serializeStruct<StructBroadcastMessage>(
          StructBroadcastMessage{
              fbae::core::AlgoLayer::Sequencer::MsgId::Broadcast, '1', sessionMsg})};
  // Serialization overhead is
  // sizeof(fbae_SequencerAlgoLayer::MsgId)+sizeof(senderPos).
  EXPECT_EQ(sizeof(fbae::core::AlgoLayer::Sequencer::MsgId) + sizeof(rank_t) +
                sizeOfEncodedSessionMsg,
            s_StructBroadcastMessage.size());
}

TEST(SerializationOverhead, SerializedBBOBBStepMsg) {
  auto sessionMsg = make_shared<SessionTest>(
      TestMessage,
      string(sizeStringInSessionMsg, 'A'));
  constexpr auto nbSessionMsgPerBatch = 1;
  std::vector<std::shared_ptr<SessionBaseClass>> v_11{
      sessionMsg};
  BatchSessionMsg batchSessionMsg_11{'1', v_11};

  constexpr auto nbBatchInStep = 1;
  std::vector<BatchSessionMsg> v_batchSessionMsg_11{
      batchSessionMsg_11};
  auto s_Step_11{serializeStruct<StepMsg>(
      StepMsg{fbae::core::AlgoLayer::BBOBB::MsgId::Step, '3', 42,
                                   55, v_batchSessionMsg_11})};
  constexpr auto sizeHeaderAndSizeVectorEncodingInStepMsg =
      sizeof(fbae::core::AlgoLayer::BBOBB::MsgId) + sizeof(rank_t) + sizeof(uint8_t) +
      sizeof(uint8_t) +
      sizeof(CEREAL_SIZE_TYPE);  // sizeof(CEREAL_SIZE_TYPE) for the encoding of
                                 // the size of the vector
  EXPECT_EQ(
      sizeHeaderAndSizeVectorEncodingInStepMsg +
          nbBatchInStep * (sizeof(BatchSessionMsg::senderPos) +
                           sizeof(CEREAL_SIZE_TYPE) +
                           nbSessionMsgPerBatch * sizeOfEncodedSessionMsg),
      s_Step_11.size());
}

TEST(SerializationOverhead, CheckMinSizeClientMessageToBroadcast) {
  auto sessionPerf = make_shared<SessionPerf>(
      PerfMeasure, 0, 0,
      std::chrono::system_clock::now(), "");
  // We store this session message in a fbae_SequencerAlgoLayer::MessagePacket
  // so that it is serialized
  auto s{serializeStruct<StructBroadcastMessage>(
      StructBroadcastMessage{
          fbae::core::AlgoLayer::Sequencer::MsgId::Broadcast, '1', sessionPerf})};

  // We compare minSizeClientMessageToBroadcast with the serialization (without
  // header of fbae_SequencerAlgoLayer::MessagePacket)
  EXPECT_EQ(minSizeClientMessageToBroadcast,
            s.size() - sizeof(fbae::core::AlgoLayer::Sequencer::MsgId) - sizeof(rank_t));
}
}  // namespace fbae::core
