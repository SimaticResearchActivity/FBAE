#ifndef FBAE_BBOBBMSG_H
#define FBAE_BBOBBMSG_H

#include "../../adaptCereal.h"
#include "../../basicTypes.h"
#include "../AlgoLayerMsg.h"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"

namespace fbae::core::AlgoLayer::BBOBB {

/**
 * @brief Message Id used by BBOBB algorithm.
 */
enum class MsgId : MsgId_t {
  Step  // Message containing a Step message sent during a wave of BBOBB
        // algorithm.
};

/**
 * @brief Structure of Step messages.
 */
struct StepMsg {
  MsgId msgId{};
  rank_t senderPos{};
  uint8_t wave;
  uint8_t step;
  std::vector<fbae::core::AlgoLayer::BatchSessionMsg> batchesBroadcast;

  // This method lets cereal know which data members to serialize
  template <class Archive>
  void serialize(Archive &archive) {
    archive(
        msgId, senderPos, wave, step,
        batchesBroadcast);  // serialize things by passing them to the archive
  }
};

}  // namespace fbae::core::AlgoLayer::BBOBB

#endif  // FBAE_BBOBBMSG_H
