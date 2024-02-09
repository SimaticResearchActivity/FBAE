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
        Broadcast = 65, // We start with a value which is displayed as a visible character in debugger
        BroadcastRequest
    };

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


}