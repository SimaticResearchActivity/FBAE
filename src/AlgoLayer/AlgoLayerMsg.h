//
// Created by simatic on 2/18/24.
//

#ifndef FBAE_ALGO_LAYER_MSG_H
#define FBAE_ALGO_LAYER_MSG_H

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "../basicTypes.h"

namespace fbae_AlgoLayer {

    /**
    * @brief Structure containing a batch of SessionMsg and the position of the sender of this batch.
    */
    struct BatchSessionMsg {
        rank_t senderPos{};
        std::vector<std::string> batchSessionMsg;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(senderPos, batchSessionMsg); // serialize things by passing them to the archive
        }
    };

}
#endif //FBAE_ALGO_LAYER_MSG_H
