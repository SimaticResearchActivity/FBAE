#pragma once

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "../../basicTypes.h"

namespace fbae_SequencerAlgoLayer
{
    /**
     * @brief Message Id used by Sequencer algorithm.
     */
    enum class MsgId : MsgId_t
    {
        Broadcast, /// Message sent by the sequencer to broadcaster(s) as a consequence of a @BroadcastRequest messag sent by a broadcaster.
        BroadcastRequest // Message sent by a broadcaster to sequencer to request the broadcast of this message.
    };

    /**
     * @brief Structure of @Broadcast and @BroadcastRequest messages.
     */
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