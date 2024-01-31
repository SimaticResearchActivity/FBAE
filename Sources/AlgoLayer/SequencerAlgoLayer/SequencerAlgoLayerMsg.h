#pragma once

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "../../basicTypes.h"

namespace fbae_SequencerAlgoLayer
{
    //---------------------------------------------------
    // Id of messages exchanged between sequencer and broadcaster(s)
    //---------------------------------------------------
    enum class MsgId : MsgId_t
    {
        // Messages sent by the sequencer to broadcaster(s)
        AckDisconnectIntent = 65, // We start with a value which be displayed as a character in debugger
        AllBroadcastersConnected,
        BroadcastMessage,
        // Messages sent by a broadcaster to the sequencer
        DisconnectIntent,
        MessageToBroadcast,
        RankInfo,
    };

    //---------------------------------------------------
    // Structure of messages sent by a broadcaster to the sequencer
    //---------------------------------------------------
    struct StructGenericMsgWithoutData
    {
        MsgId msgId{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId); // serialize things by passing them to the archive
        }
    };

    using StructAckDisconnectIntent = StructGenericMsgWithoutData;
    using StructAllBroadcastersConnected = StructGenericMsgWithoutData;

    struct StructBroadcastMessage
    {
        MsgId msgId{};
        rank_t senderRank{};
        std::string sessionMsg;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, senderRank, sessionMsg); // serialize things by passing them to the archive
        }
    };

    //---------------------------------------------------
    // Structure of messages sent by a broadcaster to the sequencer
    //---------------------------------------------------
    struct StructGenericMsgWithRank
    {
        MsgId msgId{};
        rank_t senderRank{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, senderRank); // serialize things by passing them to the archive
        }
    };

    using StructDisconnectIntent = StructGenericMsgWithRank;
    using StructRankInfo = StructGenericMsgWithRank;

    struct StructMessageToBroadcast
    {
        MsgId msgId{};
        rank_t senderRank{};
        std::string sessionMsg;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, senderRank, sessionMsg); // serialize things by passing them to the archive
        }
    };


}