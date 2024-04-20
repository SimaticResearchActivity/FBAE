//
// Created by simatic on 2/10/24.
//

#include <mutex>
#include <future>
#include "PerfMeasures.h"
#include "Logger/Logger.h"

using namespace std;
using namespace fbae_SessionLayer;

PerfMeasures::PerfMeasures(const Arguments &arguments, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer)
        : SessionLayer{arguments, rank, std::move(algoLayer)}
        , measures{static_cast<size_t>(arguments.getNbMsg() * (100 - arguments.getWarmupCooldown()) / 100) + 1}//We add +1 to avoid not allocating enough size because of rounding by default
{
}

void PerfMeasures::broadcastPerfMeasure() {
    auto logger = Logger::instance("PerfMeasures::broadcastPerfMeasure");

    if (getArguments().getVerbose()) {
        logger.info()
            << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " : Broadcast PerfMeasure (senderPos = " << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " ; msgNum = " << numPerfMeasure << ")" << Logger::endLog;
    }

    if (numPerfMeasure == getArguments().getNbMsg()*getArguments().getWarmupCooldown()/100/2)
        measures.setStartTime();
    auto sessionMsg = make_shared<SessionPerf>(SessionMsgId::PerfMeasure,
                                               getAlgoLayer()->getPosInBroadcastersGroup().value(),
                                               numPerfMeasure,
                                               std::chrono::system_clock::now(),
                                               std::string( getArguments().getSizeMsg() - minSizeClientMessageToBroadcast,
                                            0));
    getAlgoLayer()->totalOrderBroadcast(sessionMsg);
    ++numPerfMeasure;
}

void PerfMeasures::callbackDeliver(rank_t senderPos, fbae_SessionLayer::SessionMsg msg) {
    switch (msg->msgId)
    {
        using enum SessionMsgId;
        case FinishedPerfMeasures :
            processFinishedPerfMeasuresMsg(senderPos);
            break;
        case FirstBroadcast :
            processFirstBroadcastMsg(senderPos);
            break;
        case PerfMeasure :
            processPerfMeasureMsg(senderPos, msg);
            break;
        case PerfResponse :
            processPerfResponseMsg(senderPos, msg);
            break;
        default:
        {
            Logger::instanceStream(Logger::StreamType::Fatal, "PerfMeasures::callbackDeliver")
                << "Unexpected sessionMsgTyp (" << static_cast<int>(msg->msgId) << ")" << Logger::endLog;
            exit(EXIT_FAILURE);
        }
    }
}

void PerfMeasures::callbackInitDone()
{
    if (getAlgoLayer()->isBroadcastingMessages()){
        // Broadcast FirstBroadcast
        if (getArguments().getVerbose()) {
            Logger::instanceStream(Logger::StreamType::Info, "PerfMeasures::callbackInitDone")
                << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " : Broadcast FirstBroadcast (sender = " << static_cast<uint32_t>(getRank()) << ")" << Logger::endLog;
        }

        auto msg = make_shared<SessionFirstBroadcast>(SessionMsgId::FirstBroadcast);
        getAlgoLayer()->totalOrderBroadcast(msg);
    }
}

void PerfMeasures::execute()
{
    auto logger = Logger::instance("PerfMeasures::execute");
    if (getArguments().getVerbose()) {
        logger.info() << "PerfMeasures (Warning: this may not be PerfMeasures pos!) #" << static_cast<uint32_t>(getRank()) << " : Start execution" << Logger::endLog;

    }

    getAlgoLayer()->execute();
    if (getAlgoLayer()->isBroadcastingMessages()) {
        // Display statistics
        static std::mutex mtx;
        scoped_lock lock{mtx};

        logger.info() << Arguments::csvHeadline() << "," << Measures::csvHeadline() << "\n";
        logger.info() << getArguments().asCsv(getAlgoLayer()->toString(), getAlgoLayer()->getCommLayer()->toString(), to_string(getRank())) << "," << measures.asCsv() << Logger::endLog;
    }
    if (getArguments().getFrequency()  && getAlgoLayer()->isBroadcastingMessages())
        taskSendPeriodicPerfMessage.get();
    if (getArguments().getVerbose()) {
        logger.info() << "PerfMeasures (Warning: this may not be PerfMeasures pos!) #" << static_cast<uint32_t>(getRank()) << " : End of execution" << Logger::endLog;
    }

}

void PerfMeasures::processFinishedPerfMeasuresMsg(rank_t senderPos)
{
    auto logger = Logger::instance("PerfMeasures::processFinishedPerfMeasuresMsg");
    ++nbReceivedFinishedPerfMeasures;
    if (getArguments().getVerbose()) {
        logger.info() << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " : Deliver FinishedPerfMeasures from sender pos #" << static_cast<uint32_t>(senderPos) << " (nbReceivedFinishedPerfMeasures = " << nbReceivedFinishedPerfMeasures << ")" << Logger::endLog;
    }

    if (nbReceivedFinishedPerfMeasures > getAlgoLayer()->getBroadcastersGroup().size()) {
        logger.fatal() << "Delivering a FinishedPerfMeasures message while we already have received all FinishedPerfMeasures messages we were waiting for." << Logger::endLog;
        exit(EXIT_FAILURE);
    }

    if (nbReceivedFinishedPerfMeasures == getAlgoLayer()->getBroadcastersGroup().size()) {
        // All broadcasters are done doing measures ==> We can ask the getAlgoLayer() to terminate.
        getAlgoLayer() -> terminate();
    }
}

void PerfMeasures::processFirstBroadcastMsg(rank_t senderPos) {
    auto logger = Logger::instance("PerfMeasures::processFirstBroadcastMsg");
    ++nbReceivedFirstBroadcast;
    if (getArguments().getVerbose()) {
        logger.info() << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value())
               << " : Deliver FirstBroadcast from sender pos #" << static_cast<uint32_t>(senderPos)
               << " (nbReceivedFirstBroadcast = " << nbReceivedFirstBroadcast << ")" << Logger::endLog;
    }
    if (nbReceivedFirstBroadcast > getAlgoLayer()->getBroadcastersGroup().size())
    {
        logger.fatal() << "Delivering a FirstBroadcast message while we already have received all FirstBroadcast messages we were waiting for." << Logger::endLog;
        exit(EXIT_FAILURE);
    }
    if (nbReceivedFirstBroadcast == getAlgoLayer()->getBroadcastersGroup().size())
    {
        // As we have received all awaited FirstBroadcast messages, we know that @getAlgoLayer() is fully
        // operational ==> We can start our performance measures.
        if (getArguments().getFrequency())
            // We start periodic sending of PerfMessage
            taskSendPeriodicPerfMessage = std::async(std::launch::async, &PerfMeasures::sendPeriodicPerfMessage, this);
        else
            // We send a single PerfMeasure
            broadcastPerfMeasure();
    }
}

void PerfMeasures::processPerfMeasureMsg(rank_t senderPos, const fbae_SessionLayer::SessionMsg &sessionMsg) {
    auto logger = Logger::instance("PerfMeasures::processPerfMeasureMsg");

    auto nakedSessionMsg = dynamic_cast<SessionPerf*>(sessionMsg.get());
    if (getArguments().getVerbose()) {
        logger.info() << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " : Deliver PerfMeasure from sender pos #" << static_cast<uint32_t>(senderPos) << " (senderPos = " << static_cast<uint32_t>(nakedSessionMsg->senderPos) << " ; msgNum = " << nakedSessionMsg->msgNum << ")" << Logger::endLog;
    }

    measures.addNbBytesDelivered(getArguments().getSizeMsg());
    // We check which process must send the PerfResponse. The formula hereafter guarantees that first PerfMeasure is
    // answered by successor of sender process, second PerfMeasure message is answered by successor of the successor of
    // sender process, etc.
    if ((nakedSessionMsg->senderPos + nakedSessionMsg->msgNum) % getAlgoLayer()->getBroadcastersGroup().size() ==
        getAlgoLayer()->getPosInBroadcastersGroup().value())
    {
        // Current process must broadcast PerfResponse message for this PerfMeasure message.
        if (getArguments().getVerbose()) {
            logger.info() << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " : Broadcast PerfResponse by sender pos #" << static_cast<uint32_t>(getRank()) << " (perfMeasureSenderPos = " << static_cast<uint32_t>(nakedSessionMsg->senderPos) << " ; perfMeasureMsgNum = " << nakedSessionMsg->msgNum << ")" << Logger::endLog;
        }
        sessionMsg->msgId = SessionMsgId::PerfResponse;
        getAlgoLayer()->totalOrderBroadcast(sessionMsg);
    }
}

void PerfMeasures::processPerfResponseMsg(rank_t senderPos, const fbae_SessionLayer::SessionMsg &sessionMsg) {
    auto logger = Logger::instance("PerfMeasures::processPerfResponseMsg");

    auto nakedSessionMsg = dynamic_cast<SessionPerf*>(sessionMsg.get());
    if (getArguments().getVerbose()) {
        logger.info() << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " : Deliver PerfResponse from sender pos #" << static_cast<uint32_t>(senderPos) << " (perfMeasureSenderPos = " << static_cast<uint32_t>(nakedSessionMsg->senderPos) << " ; perfMeasureMsgNum = " << nakedSessionMsg->msgNum << ")" << Logger::endLog;
    }

    measures.addNbBytesDelivered(getArguments().getSizeMsg());
    chrono::duration<double, std::milli> elapsed = std::chrono::system_clock::now() - nakedSessionMsg->sendTime;
    if (nakedSessionMsg->senderPos == getAlgoLayer()->getPosInBroadcastersGroup().value())
    {
        if (nbReceivedPerfResponseForSelf >= getArguments().getNbMsg()) {
            logger.warn() << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " : Deliver too many PerfResponse for self" << Logger::endLog;
            return;
        }
        ++nbReceivedPerfResponseForSelf;
        if (nakedSessionMsg->msgNum >= getArguments().getNbMsg() * getArguments().getWarmupCooldown() / 100 / 2
            && nakedSessionMsg->msgNum < getArguments().getNbMsg() - getArguments().getNbMsg() * getArguments().getWarmupCooldown() / 100 / 2)
            measures.add(elapsed);
        if (nakedSessionMsg->msgNum == getArguments().getNbMsg() - getArguments().getNbMsg() * getArguments().getWarmupCooldown() / 100 / 2 - 1)
            measures.setStopTime();
        if (nakedSessionMsg->msgNum < getArguments().getNbMsg() - 1)
        {
            if (!getArguments().getFrequency())
                // As we do not send periodic PerfMessage, we send another PerfMessage
                broadcastPerfMeasure();
        }
        else
        {
            // Process is done with sending PerfMeasure messages. It tells it is done to all broadcasters
            if (getArguments().getVerbose()) {
                logger.info() << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << " : Broadcast FinishedPerfMeasures by sender pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcastersGroup().value()) << Logger::endLog;
            }

            auto sessionMsgToSend = make_shared<SessionFinishedPerfMeasures>(SessionMsgId::FinishedPerfMeasures);
            getAlgoLayer()->totalOrderBroadcast(sessionMsgToSend);
        }
    }
}

void PerfMeasures::sendPeriodicPerfMessage() {
    getAlgoLayer()->batchRegisterThreadForFullBatchCtrl();
    constexpr std::chrono::duration<double, std::milli> sleepDuration{5ms};
    constexpr double nbMillisecondsPerSecond{ 1'000.0 };
    const auto freq{ getArguments().getFrequency() };
    auto startSending{ std::chrono::system_clock::now() };
    while(true) {
        auto elapsedPeriod{ duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startSending) };
        // Broadcast PerfMeasure messages until we reach the desired frequency
        while (numPerfMeasure < freq * static_cast<double>(elapsedPeriod.count()) / nbMillisecondsPerSecond) {
            broadcastPerfMeasure();
            if (numPerfMeasure >= getArguments().getNbMsg())
                return;
        }
        std::this_thread::sleep_for(sleepDuration);
    }
}
