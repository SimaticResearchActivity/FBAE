//
// Created by simatic on 2/10/24.
//

#include "PerfMeasures.h"

#include <future>
#include <mutex>
#include <syncstream>

using namespace std;
using namespace fbae::core;

namespace fbae::core::SessionLayer::PerfMeasures {

PerfMeasures::PerfMeasures(const Arguments &arguments, rank_t rank,
                           std::unique_ptr<AlgoLayer::AlgoLayer> algoLayer)
    : SessionLayer{arguments, rank, std::move(algoLayer),
                   "fbae.core.SessionLayer.PerfMeasures"},
      measures{static_cast<size_t>(arguments.getNbMsg() *
                                   (100 - arguments.getWarmupCooldown()) /
                                   100) +
               1}
// We add +1 to avoid not allocating enough size because of rounding by default
{}

void PerfMeasures::broadcastPerfMeasure() {
  LOG4CXX_INFO_FMT(getSessionLogger(),
                   "PerfMeasures pos #{:d} : Broadcast PerfMeasure (senderPos "
                   "= {:d} ; msgNum = {:d})",
                   getAlgoLayer()->getPosInBroadcastersGroup().value(),
                   getAlgoLayer()->getPosInBroadcastersGroup().value(),
                   numPerfMeasure);

  if (numPerfMeasure ==
      getArguments().getNbMsg() * getArguments().getWarmupCooldown() / 100 / 2)
    measures.setStartTime();
  auto sessionMsg = make_shared<SessionPerf>(
      SessionMsgId::PerfMeasure,
      getAlgoLayer()->getPosInBroadcastersGroup().value(), numPerfMeasure,
      std::chrono::system_clock::now(),
      std::string(getArguments().getSizeMsg() - minSizeClientMessageToBroadcast,
                  0));
  getAlgoLayer()->totalOrderBroadcast(sessionMsg);
  ++numPerfMeasure;
}

void PerfMeasures::callbackDeliver(rank_t senderPos,
                                   SessionMsg msg) {
  switch (msg->msgId) {
    using enum SessionMsgId;
    case FinishedPerfMeasures:
      processFinishedPerfMeasuresMsg(senderPos);
      break;
    case FirstBroadcast:
      processFirstBroadcastMsg(senderPos);
      break;
    case PerfMeasure:
      processPerfMeasureMsg(senderPos, msg);
      break;
    case PerfResponse:
      processPerfResponseMsg(senderPos, msg);
      break;
    default: {
      LOG4CXX_FATAL_FMT(getSessionLogger(), "Unexpected sessionMsgTyp ({:d})",
                        static_cast<uint32_t>(msg->msgId));
      exit(EXIT_FAILURE);
    }
  }
}

void PerfMeasures::callbackInitDone() {
  if (getAlgoLayer()->isBroadcastingMessages()) {
    // Broadcast FirstBroadcast
    LOG4CXX_INFO_FMT(
        getSessionLogger(),
        "PerfMeasures pos #{:d} : Broadcast FirstBroadcast (sender = {:d})",
        getAlgoLayer()->getPosInBroadcastersGroup().value(), getRank());

    auto msg = make_shared<SessionFirstBroadcast>(SessionMsgId::FirstBroadcast);
    getAlgoLayer()->totalOrderBroadcast(msg);
  }
}

void PerfMeasures::execute() {
  LOG4CXX_INFO_FMT(getSessionLogger(),
                   "PerfMeasures (Warning: this may not be PerfMeasures pos!) "
                   "#{:d} : Start execution",
                   getRank());

  getAlgoLayer()->execute();
  if (getAlgoLayer()->isBroadcastingMessages()) {
    // Display statistics
    static std::mutex mtx;
    scoped_lock lock{mtx};

    std::osyncstream synced_out(std::cout);
    synced_out << Arguments::csvHeadline() << "," << Measures::csvHeadline()
               << endl;

    synced_out << getArguments().asCsv(
                      getAlgoLayer()->toString(),
                      getAlgoLayer()->getCommLayer()->toString(),
                      to_string(getRank()))
               << "," << measures.asCsv() << endl;
  }
  if (getArguments().getFrequency() && getAlgoLayer()->isBroadcastingMessages())
    taskSendPeriodicPerfMessage.get();
  LOG4CXX_INFO_FMT(getSessionLogger(),
                   "PerfMeasures (Warning: this may not be PerfMeasures pos!) "
                   "#{:d} : End of execution",
                   getRank());
}

void PerfMeasures::processFinishedPerfMeasuresMsg(rank_t senderPos) {
  ++nbReceivedFinishedPerfMeasures;
  LOG4CXX_INFO_FMT(getSessionLogger(),
                   "PerfMeasures pos #{:d} : Deliver FinishedPerfMeasures from "
                   "sender pos #{:d}, (nbReceivedFinishedPerfMeasures = {:d})",
                   getAlgoLayer()->getPosInBroadcastersGroup().value(),
                   senderPos, nbReceivedFinishedPerfMeasures);

  if (nbReceivedFinishedPerfMeasures >
      getAlgoLayer()->getBroadcastersGroup().size()) {
    LOG4CXX_FATAL(
        getSessionLogger(),
        "Delivering a FinishedPerfMeasures message while we already have "
        "received all FinishedPerfMeasures messages we were waiting for.");
    exit(EXIT_FAILURE);
  }

  if (nbReceivedFinishedPerfMeasures ==
      getAlgoLayer()->getBroadcastersGroup().size()) {
    // All broadcasters are done doing measures ==> We can ask the
    // getAlgoLayer() to terminate.
    getAlgoLayer()->terminate();
  }
}

void PerfMeasures::processFirstBroadcastMsg(rank_t senderPos) {
  ++nbReceivedFirstBroadcast;
  LOG4CXX_INFO_FMT(getSessionLogger(),
                   "PerfMeasures pos #{:d} : Deliver FirstBroadcast from "
                   "sender pos #{:d}, (nbReceivedFirstBroadcast = {:d})",
                   getAlgoLayer()->getPosInBroadcastersGroup().value(),
                   senderPos, nbReceivedFirstBroadcast);

  if (nbReceivedFirstBroadcast >
      getAlgoLayer()->getBroadcastersGroup().size()) {
    LOG4CXX_FATAL(getSessionLogger(),
                  "Delivering a FirstBroadcast message while we already have "
                  "received all FirstBroadcast messages we were waiting for.");

    exit(EXIT_FAILURE);
  }
  if (nbReceivedFirstBroadcast ==
      getAlgoLayer()->getBroadcastersGroup().size()) {
    // As we have received all awaited FirstBroadcast messages, we know that
    // @getAlgoLayer() is fully operational ==> We can start our performance
    // measures.
    if (getArguments().getFrequency())
      // We start periodic sending of PerfMessage
      taskSendPeriodicPerfMessage = std::async(
          std::launch::async, &PerfMeasures::sendPeriodicPerfMessage, this);
    else
      // We send a single PerfMeasure
      broadcastPerfMeasure();
  }
}

void PerfMeasures::processPerfMeasureMsg(
    rank_t senderPos, const SessionMsg &sessionMsg) {
  auto nakedSessionMsg = dynamic_cast<SessionPerf *>(sessionMsg.get());

  LOG4CXX_INFO_FMT(getSessionLogger(),
                   "PerfMeasures pos #{:d} : Deliver PerfMeasure from sender "
                   "pos #{:d}, (senderPos = {:d} ; msgNum = {:d})",
                   getAlgoLayer()->getPosInBroadcastersGroup().value(),
                   senderPos, nakedSessionMsg->senderPos,
                   nakedSessionMsg->msgNum);

  measures.addNbBytesDelivered(getArguments().getSizeMsg());
  // We check which process must send the PerfResponse. The formula hereafter
  // guarantees that first PerfMeasure is answered by successor of sender
  // process, second PerfMeasure message is answered by successor of the
  // successor of sender process, etc.
  if ((nakedSessionMsg->senderPos + nakedSessionMsg->msgNum) %
          getAlgoLayer()->getBroadcastersGroup().size() ==
      getAlgoLayer()->getPosInBroadcastersGroup().value()) {
    // Current process must broadcast PerfResponse message for this PerfMeasure
    // message.
    LOG4CXX_INFO_FMT(
        getSessionLogger(),
        "PerfMeasures pos #{:d} : Broadcast PerfResponse by sender pos #{:d} "
        "(perfMeasureSenderPos = {:d} ; perfMeasureMsgNum =  {:d})",
        getAlgoLayer()->getPosInBroadcastersGroup().value(), getRank(),
        senderPos, nakedSessionMsg->msgNum);

    sessionMsg->msgId = SessionMsgId::PerfResponse;
    getAlgoLayer()->totalOrderBroadcast(sessionMsg);
  }
}

void PerfMeasures::processPerfResponseMsg(
    rank_t senderPos, const SessionMsg &sessionMsg) {
  auto nakedSessionMsg = dynamic_cast<SessionPerf *>(sessionMsg.get());
  LOG4CXX_INFO_FMT(
      getSessionLogger(),
      "PerfMeasures pos #{:d} : Deliver PerfResponse from sender pos #{:d} "
      "(perfMeasureSenderPos = {:d} ; perfMeasureMsgNum =  {:d})",
      getAlgoLayer()->getPosInBroadcastersGroup().value(), senderPos,
      nakedSessionMsg->senderPos, nakedSessionMsg->msgNum);

  measures.addNbBytesDelivered(getArguments().getSizeMsg());
  chrono::duration<double, std::milli> elapsed =
      std::chrono::system_clock::now() - nakedSessionMsg->sendTime;
  if (nakedSessionMsg->senderPos ==
      getAlgoLayer()->getPosInBroadcastersGroup().value()) {
    if (nbReceivedPerfResponseForSelf >= getArguments().getNbMsg()) {
      LOG4CXX_WARN_FMT(
          getSessionLogger(),
          "PerfMeasures pos #{} : Deliver too many PerfResponse for self",
          getAlgoLayer()->getPosInBroadcastersGroup().value());
      return;
    }
    ++nbReceivedPerfResponseForSelf;
    if (nakedSessionMsg->msgNum >= getArguments().getNbMsg() *
                                       getArguments().getWarmupCooldown() /
                                       100 / 2 &&
        nakedSessionMsg->msgNum <
            getArguments().getNbMsg() - getArguments().getNbMsg() *
                                            getArguments().getWarmupCooldown() /
                                            100 / 2)
      measures.add(elapsed);
    if (nakedSessionMsg->msgNum == getArguments().getNbMsg() -
                                       getArguments().getNbMsg() *
                                           getArguments().getWarmupCooldown() /
                                           100 / 2 -
                                       1)
      measures.setStopTime();
    if (nakedSessionMsg->msgNum < getArguments().getNbMsg() - 1) {
      if (!getArguments().getFrequency())
        // As we do not send periodic PerfMessage, we send another PerfMessage
        broadcastPerfMeasure();
    } else {
      // Process is done with sending PerfMeasure messages. It tells it is done
      // to all broadcasters
      LOG4CXX_INFO_FMT(getSessionLogger(),
                       "PerfMeasures pos #{:d} : Broadcast "
                       "FinishedPerfMeasures by sender pos #{:d}",
                       getAlgoLayer()->getPosInBroadcastersGroup().value(),
                       getAlgoLayer()->getPosInBroadcastersGroup().value());

      auto sessionMsgToSend = make_shared<SessionFinishedPerfMeasures>(
          SessionMsgId::FinishedPerfMeasures);
      getAlgoLayer()->totalOrderBroadcast(sessionMsgToSend);
    }
  }
}

void PerfMeasures::sendPeriodicPerfMessage() {
  getAlgoLayer()->batchRegisterThreadForFullBatchCtrl();
  constexpr std::chrono::duration<double, std::milli> sleepDuration{5ms};
  constexpr double nbMillisecondsPerSecond{1'000.0};
  const auto freq{getArguments().getFrequency()};
  auto startSending{std::chrono::system_clock::now()};
  while (true) {
    auto elapsedPeriod{duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - startSending)};
    // Broadcast PerfMeasure messages until we reach the desired frequency
    while (numPerfMeasure < freq * static_cast<double>(elapsedPeriod.count()) /
                                nbMillisecondsPerSecond) {
      broadcastPerfMeasure();
      if (numPerfMeasure >= getArguments().getNbMsg()) return;
    }
    std::this_thread::sleep_for(sleepDuration);
  }
}

}  // namespace fbae::core::SessionLayer::PerfMeasures
