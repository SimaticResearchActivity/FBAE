//
// Created by simatic on 2/5/24.
//

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "../Sources/basicTypes.h"
#include "../Sources/msgTemplates.h"
#include "../Sources/SessionLayer/SessionLayerMsg.h"
#include "../Sources/AlgoLayer/SequencerAlgoLayer/SequencerAlgoLayerMsg.h"
#include "../Sources/AlgoLayer/BBOBBAlgoLayer/BBOBBAlgoLayerMsg.h"
#include <vector>

using namespace std;

struct PseudoSessionMsg
{
    fbae_SessionLayer::SessionMsgId msgId{};
    std::string payload;

    // This method lets cereal know which data members to serialize
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(msgId, payload); // serialize things by passing them to the archive
    }
};

int main()
{
    constexpr size_t payloadSizeSessionMsg = 20;

    auto s_PseudoSessionMsg {serializeStruct<PseudoSessionMsg>(PseudoSessionMsg{fbae_SessionLayer::SessionMsgId::FinishedPerfMeasures,
                                                                                string(
                                                                                        payloadSizeSessionMsg,
                                                                                        'A')})};

    cout << "Size PseudoSessionMsg = " << s_PseudoSessionMsg.size() - payloadSizeSessionMsg << " + payloadSize at Session level\n";

    // Overhead of fbae_SequencerAlgoLayer::StructBroadcastMessage
    auto s_StructBroadcastMessage {serializeStruct<fbae_SequencerAlgoLayer::StructBroadcastMessage>(fbae_SequencerAlgoLayer::StructBroadcastMessage{fbae_SequencerAlgoLayer::MsgId::Broadcast,
                                                                                                                                                    '1',
                                                                                                                                                    s_PseudoSessionMsg})};
    cout << "Size fbae_SequencerAlgoLayer::StructBroadcastMessage = " << s_StructBroadcastMessage.size() - payloadSizeSessionMsg << " + payloadSize at Session level\n";
    // Overhead of fbae_BBOBBAlgoLayer::StepMsg
    std::vector<std::string> v_11{s_PseudoSessionMsg};
    fbae_BBOBBAlgoLayer::BatchSessionMsg batchSessionMsg_11 {
            '1',
            v_11};
    auto s_batchSessionMsg_11 {serializeStruct<fbae_BBOBBAlgoLayer::BatchSessionMsg>(batchSessionMsg_11)};
    constexpr auto sizeHeaderAndSizeVectorEncodingInBatchSessionMsg = sizeof(fbae_BBOBBAlgoLayer::MsgId) + sizeof(size_t); // sizeof(size_t) for the encoding of the size of the vector
    cout << "Size fbae_BBOBBAlgoLayer::BatchSessionMsg = "
         << sizeHeaderAndSizeVectorEncodingInBatchSessionMsg
         << " + nbSessionMsgPerBatch x ( "
         << s_batchSessionMsg_11.size() - sizeHeaderAndSizeVectorEncodingInBatchSessionMsg - payloadSizeSessionMsg
         << " + payloadSize at Session level )\n";

    std::vector<fbae_BBOBBAlgoLayer::BatchSessionMsg> v_batchSessionMsg_11{batchSessionMsg_11};
    auto s_Step_11 {serializeStruct<fbae_BBOBBAlgoLayer::StepMsg>(fbae_BBOBBAlgoLayer::StepMsg{fbae_BBOBBAlgoLayer::MsgId::Step,
                                                                                                '3',
                                                                                                42,
                                                                                                55,
                                                                                                v_batchSessionMsg_11})};
    constexpr auto sizeHeaderAndSizeVectorEncodingInStepMsg = sizeof(fbae_BBOBBAlgoLayer::MsgId) + sizeof(rank_t) + sizeof(int) + sizeof(int) + sizeof(size_t); // sizeof(size_t) for the encoding of the size of the vector
    cout << "Size fbae_BBOBBAlgoLayer::StepMsg = "
         << sizeHeaderAndSizeVectorEncodingInStepMsg
         << " + nbBatchInStep x "
         << " [ "
         << sizeHeaderAndSizeVectorEncodingInBatchSessionMsg
         << " + nbSessionMsgPerBatch x ( "
         << s_batchSessionMsg_11.size() - sizeHeaderAndSizeVectorEncodingInBatchSessionMsg - payloadSizeSessionMsg
         << " + payloadSize at Session level ) ]\n";
}


