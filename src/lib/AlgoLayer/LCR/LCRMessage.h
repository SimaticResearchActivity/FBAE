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
         * @brief A flag describing whether or not this message is stable and thus
         * ready for delivery.
         */
        bool isStable = false;
        /**
         * @brief The `scalar` clock of the current site (ie the value of the vector
         * clock of the current site at the position of the current site).
         */
        lcr_clock_t clock;
        /**
         * @brief The actual message that was sent.
         */
        fbae_SessionLayer::SessionMsg sessionMessage;

        template<class Archive> void serialize(Archive& archive) {
            // serialize things by passing them to the archive
            archive(messageId, senderRank, clock, sessionMessage, isStable);
        }
    };
}