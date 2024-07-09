#pragma once

#include "../AlgoLayer.h"
#include "LCRMessage.h"
#include "LCRTypedefs.h"

// Algorithm described in the following paper:
// https://infoscience.epfl.ch/record/149218/files/paper.pdf?version=2 Page 9
// gives pseudocode for the algorithm. Differences: messages do not contain the
// entire vector clock of the site that sent it, only the value for its own
// rank.

namespace fbae::core::AlgoLayer::LCR {

/** @brief The LCR Algorithm. */
class LCR : public AlgoLayer {
 public:
  explicit LCR(std::unique_ptr<fbae::core::CommLayer::CommLayer> commLayer) noexcept;

  void callbackReceive(std::string &&serializedMessagePacket) noexcept override;
  void execute() noexcept override;
  void terminate() noexcept override;
  void totalOrderBroadcast(
      const fbae::core::SessionLayer::SessionMsg &sessionMsg) noexcept override;

  std::string toString() noexcept override;

 private:
  /**
   * @brief Internal function used to initialize the vector clock.
   */
  void initializeVectorClock() noexcept;

  /**
   * @brief Internal function used to try delivering a message to
   * the session layer.
   */
  void tryDeliver() noexcept;

  /**
   * @brief Internal function for handling reception of a message from another
   * site.
   * @param message: The message we received that want to handle.
   * @return Optionally returns the message to send to the successor in the ring
   * of sites.
   */
  std::optional<MessagePacket> handleMessageReceive(
      MessagePacket message) noexcept;

  /**
   * @brief Internal function for handling reception of an acknowledgement from
   * another site.
   * @param message: The acknowledgment message we received that want to handle.
   * @return Optionally returns the acknowledgement message to send to the
   * successor in the ring of sites.
   */
  std::optional<MessagePacket> handleAcknowledgmentReceive(
      MessagePacket message) noexcept;

  /**
   * @brief The internal vector clock of the site.
   */
  std::vector<LCRClock_t> vectorClock;

  /**
   * @brief The pending list of messages of the site.
   */
  std::vector<MessagePacket> pending;
};

}  // namespace fbae::core::AlgoLayer::LCR
