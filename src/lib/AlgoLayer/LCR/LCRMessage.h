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
         * @brief An acknowledgement message we are forwarding to another site.
         * This is a message a site should receive after having already
         * received a classic "Message" message. Once received, the site
         * knows this message is ready to be delivered.
         */
        Acknowledgement,

        /**
         * @brief A classic message we are forwarding to another site.
         * When receiving this message, a site is not expected to have
         * received it's content yet, and is not ready to deliver its
         * content to the session layer, as not all sites are guaranteed
         * to have received the message yet.
         */
        Message,
    };

    /**
     * @brief The structured message packet we want to send to another site.
     * Contains all of the information we want to convey.
     */
    struct StructBroadcastMessage {
        /**
         * @brief the message type. See MessageId enum for
         * more information.
         */
        MessageId messageId = MessageId::Message;

        /**
         * @brief The rank of the site that sent this message
         * in the first place (the one on which was called
         * the totalOrderBroadcast method).
         */
        rank_t senderRank;

        /**
         * @brief The value of the sender's vector clock indexed
         * at its own rank, or more simply the "time" at which
         * this message was sent.
         */
        lcrClock_t clock;

        /**
         * @brief The actual message that was sent from the session
         * layer.
         */
        fbae_SessionLayer::SessionMsg sessionMessage;


        /**
         * @brief flag used to check if the message is stable.
         * A message is stable if it can be delivered safely
         * to the session layer (which means, if we have the
         * guarantee that all sites have acquired this message).
         */
        bool isStable = false;

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

