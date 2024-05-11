#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"

namespace fbae_TrainsAlgoLayer {

    struct MessagePacket {

        /**
        * @brief The rank of the site that sent this message
        * in the first place (the one on which was called
        * the totalOrderBroadcast method).
        */
        rank_t senderRank;

        /**
        * @brief The actual message that was sent from the session
        * layer.
        */
        fbae_SessionLayer::SessionMsg sessionMessage;

        template<class Archive> void serialize(Archive& archive) {
            // serialize things by passing them to the archive
            archive(sessionMessage); //On met tous les paramètres qu'on ajoute dans archive
        }

    };


}