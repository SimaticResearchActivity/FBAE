#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"

namespace fbae_LCRAlgoLayer {
    enum class MessageId: MsgId_t {
        Acknowledgement,
        Message,
    };

    struct StructBroadcastMessage {
        MessageId messageId {};
        rank_t senderRank {};
        rank_t currentRank {};
        bool isStable = false;
        std::vector<uint32_t> vectorClock;
        fbae_SessionLayer::SessionMsg sessionMessage;

        template<class Archive> void serialize(Archive& archive) {
            // serialize things by passing them to the archive
            archive(messageId, senderRank, vectorClock, sessionMessage, isStable);
        }
    };
}