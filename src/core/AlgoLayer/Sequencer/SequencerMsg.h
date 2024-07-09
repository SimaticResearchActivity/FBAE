#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"
#include "../../adaptCereal.h"
#include "../../basicTypes.h"
#include "cereal/archives/binary.hpp"

namespace fbae::core::AlgoLayer::Sequencer {
/**
 * @brief Message Id used by Sequencer algorithm.
 */
enum class MsgId : MsgId_t {
  Broadcast,        /// Message sent by the sequencer to broadcaster(s) as a
                    /// consequence of a @BroadcastRequest message sent by a
                    /// broadcaster.
  BroadcastRequest  // Message sent by a broadcaster to sequencer to request the
                    // broadcast of this message.
};

/**
 * @brief Structure of @Broadcast and @BroadcastRequest messages.
 */
struct StructBroadcastMessage {
  MsgId msgId{};
  rank_t senderPos{};
  fbae::core::SessionLayer::SessionMsg sessionMsg;

  // This method lets cereal know which data members to serialize
  template <class Archive>
  void serialize(Archive& archive) {
    archive(msgId, senderPos,
            sessionMsg);  // serialize things by passing them to the archive
  }
};

}  // namespace fbae::core::AlgoLayer::Sequencer