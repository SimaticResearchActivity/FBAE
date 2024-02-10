#ifndef FBAE_BBOBBMSG_H
#define FBAE_BBOBBMSG_H

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "../../basicTypes.h"

namespace fbae_BBOBBAlgoLayer {

    /**
     * @brief Message Id used by BBOBB algorithm.
     */
    enum class MsgId : MsgId_t {
        Step // Message containing a Step message sent during a wave of BBOBB algorithm.
    };

    /**
     * @brief Structure containing a batch of SessionMsg and the rank of the sender of this batch.
     */
    struct BatchSessionMsg {
        rank_t senderRank{};
        std::vector<std::string> batchSessionMsg;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(senderRank, batchSessionMsg); // serialize things by passing them to the archive
        }
    };

    /**
     * @brief Structure of Step messages.
     */
    struct StepMsg {
        MsgId msgId{};
        rank_t senderRank{};
        int wave;
        int step;
        std::vector<BatchSessionMsg> batchesBroadcast;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId, senderRank, wave, step, batchesBroadcast); // serialize things by passing them to the archive
        }
    };

}

#endif //FBAE_BBOBBMSG_H
