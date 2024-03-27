#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"
#include "LCRTypedefs.h"

namespace fbae_LCRAlgoLayer {
    /**
     * @brief The message ID that allows us to differentiate between
     * a classic message and an acknowledgement message.
     */
    enum class MessageId: MsgId_t {
        /**
         * @brief A classic message we are forwarding to another site.
         */
        Acknowledgement,
        /**
         * @brief An acknowledgement message we are forwarding to another site.
         */
        Message,
    };

    /**
     * @brief The structured message packet we want to send to another site.
     * Contains all of the information we want to convey.
     */
    struct StructBroadcastMessage {
        /**
         * @brief the message type: classic message or acknowledgment message.
         */
        MessageId messageId {};
        /**
         * @brief The rank of the site that sent this message in the first place
         * (the one on which was called "totalOrderBroadcast")
         */
        rank_t senderRank {};
        /**
         * @brief The value of the sender's vector clock indexed at its own rank.
         */
        lcr_clock_t clock;
        /**
         * @brief The actual message that was sent.
         */
        fbae_SessionLayer::SessionMsg sessionMessage;

        bool isStable;

        template<class Archive> void serialize(Archive& archive) {
            // serialize things by passing them to the archive
            archive(messageId, senderRank, clock, sessionMessage, isStable);
        }

        friend std::ostream &operator<<(std::ostream &os, const StructBroadcastMessage &message) {
            return os <<(static_cast<bool>(message.messageId) ? "Message" : "Acknowledgement")
                      << " from "
                      << static_cast<uint32_t>(message.senderRank);
        }
    };
}

