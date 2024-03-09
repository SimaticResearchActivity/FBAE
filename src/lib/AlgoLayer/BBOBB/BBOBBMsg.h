#ifndef FBAE_BBOBBMSG_H
#define FBAE_BBOBBMSG_H

#include "../../adaptCereal.h"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"
#include "../../basicTypes.h"
#include "../AlgoLayerMsg.h"

namespace fbae_BBOBBAlgoLayer {

    /**
     * @brief Message Id used by BBOBB algorithm.
     */
    enum class MsgId : MsgId_t {
        Step // Message containing a Step message sent during a wave of BBOBB algorithm.
    };

    /**
     * @brief Structure of Step messages.
     */
    struct StepMsg {
        MsgId msgId{};
        rank_t senderPos{};
        uint8_t wave;
        uint8_t step;
        std::vector<fbae_AlgoLayer::BatchSessionMsg> batchesBroadcast;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId, senderPos, wave, step, batchesBroadcast); // serialize things by passing them to the archive
        }
    };

}

#endif //FBAE_BBOBBMSG_H
