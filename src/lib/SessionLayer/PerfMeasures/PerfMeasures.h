//
// Created by simatic on 2/10/24.
//

#ifndef FBAE_PERF_MEASURES_H
#define FBAE_PERF_MEASURES_H

#include <future>
#include <memory>

#include "../../AlgoLayer/AlgoLayer.h"
#include "../SessionLayer.h"
#include "Measures.h"

class PerfMeasures : public SessionLayer {
 public:
  PerfMeasures(const Arguments &arguments, rank_t rank,
               std::unique_ptr<AlgoLayer> algoLayer);

  void callbackDeliver(rank_t senderPos,
                       fbae_SessionLayer::SessionMsg msg) override;
  void callbackInitDone() override;
  void execute() override;

 private:
  Measures measures;
  int32_t numPerfMeasure{0};
  int32_t nbReceivedPerfResponseForSelf{0};
  size_t nbReceivedFirstBroadcast{0};
  size_t nbReceivedFinishedPerfMeasures{0};
  std::future<void> taskSendPeriodicPerfMessage;

  /**
   * @brief Broadcasts a @PerfMeasure message with @msgNum incremented by 1.
   */
  void broadcastPerfMeasure();

  /**
   * @brief Called by @callbackDeliver to process @FinishedPerfMeasures message
   * @param senderPos Rank of message sender.
   */
  void processFinishedPerfMeasuresMsg(rank_t senderPos);

  /**
   * @brief Called by @callbackDeliver to process @FirstBroadcast message
   * @param senderPos Rank of message sender.
   */
  void processFirstBroadcastMsg(rank_t senderPos);

  /**
   * @brief Called by @callbackDeliver to process @PerfMeasure message
   * @param senderPos Rank of message sender.
   * @param sessionMsg Message to process.
   */
  void processPerfMeasureMsg(rank_t senderPos,
                             const fbae_SessionLayer::SessionMsg &sessionMsg);

  /**
   * @brief Called by @callbackDeliver to process @PerfMeasure message
   * @param senderPos Rank of message sender.
   * @param sessionMsg Message to process.
   */
  void processPerfResponseMsg(rank_t senderPos,
                              const fbae_SessionLayer::SessionMsg &sessionMsg);

  /**
   * @brief Thread to send PerfMessage at @Param::frequency per second.
   */
  void sendPeriodicPerfMessage();
};

#endif  // FBAE_PERF_MEASURES_H
