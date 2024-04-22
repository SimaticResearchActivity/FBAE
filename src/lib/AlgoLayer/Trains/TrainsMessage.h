#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"

namespace fbae_TrainsAlgoLayer {

    struct MessagePacket {

        fbae_SessionLayer::SessionMsg sessionMessage;

        template<class Archive> void serialize(Archive& archive) {
            // serialize things by passing them to the archive
            archive(sessionMessage); //On met tous les paramètres qu'on ajoute dans archive
        }

    };


}