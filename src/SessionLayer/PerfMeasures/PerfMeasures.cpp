//
// Created by simatic on 2/10/24.
//

#include <iostream>
#include <mutex>
#include <future>
#include "PerfMeasures.h"
#include "../SessionLayerMsg.h"
#include "../../msgTemplates.h"

using namespace std;
using namespace fbae_SessionLayer;

PerfMeasures::PerfMeasures(const Arguments &arguments, rank_t rank, std::unique_ptr<AlgoLayer> algoLayer)
        : SessionLayer{arguments, rank, std::move(algoLayer)}
        , measures{static_cast<size_t>(arguments.getNbMsg() * (100 - arguments.getWarmupCooldown()) / 100) + 1}//We add +1 to avoid not allocating enough size because of rounding by default
{
}

void PerfMeasures::broadcastPerfMeasure() {
    if (getArguments().getVerbose())
        cout << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Broadcast PerfMeasure (senderPos = " << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " ; msgNum = " << numPerfMeasure << ")\n";
    if (numPerfMeasure == getArguments().getNbMsg()*getArguments().getWarmupCooldown()/100/2)
        measures.setStartTime();
    auto s {serializeStruct<SessionPerfMeasure>(SessionPerfMeasure{SessionMsgId::PerfMeasure,
                                                                   getAlgoLayer()->getPosInBroadcasters(),
                                                                   numPerfMeasure,
                                                                   std::chrono::system_clock::now(),
                                                                   std::string(
                                                                           getArguments().getSizeMsg() -
                                                                           minSizeClientMessageToBroadcast,
                                                                           0)})};
    getAlgoLayer()->totalOrderBroadcast(std::move(s));
    ++numPerfMeasure;
}

void PerfMeasures::callbackDeliver(rank_t senderPos, std::string && msg) {
    switch (auto sessionMsgTyp{ static_cast<SessionMsgId>(msg[0]) }; sessionMsgTyp)
    {
        using enum SessionMsgId;
        case FinishedPerfMeasures :
            processFinishedPerfMeasuresMsg(senderPos);
            break;
        case FirstBroadcast :
            processFirstBroadcastMsg(senderPos);
            break;
        case PerfMeasure :
            processPerfMeasureMsg(senderPos, std::move(msg));
            break;
        case PerfResponse :
            processPerfResponseMsg(senderPos, std::move(msg));
            break;
        default:
        {
            cerr << "ERROR: Unexpected sessionMsgTyp (" << static_cast<int>(sessionMsgTyp) << ")\n";
            exit(EXIT_FAILURE);
        }
    }
}

void PerfMeasures::callbackInitDone() const
{
    if (getAlgoLayer()->isBroadcastingMessage()){
        // Broadcast FirstBroadcast
        if (getArguments().getVerbose())
            cout << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Broadcast FirstBroadcast (sender = " << static_cast<uint32_t>(getRank()) << ")\n";
        auto s {serializeStruct<SessionFirstBroadcast>(SessionFirstBroadcast{SessionMsgId::FirstBroadcast})};
        getAlgoLayer()->totalOrderBroadcast(std::move(s));
    }
}

void PerfMeasures::execute()
{
    if (getArguments().getVerbose())
        cout << "PerfMeasures (Warning: this may not be PerfMeasures pos!) #" << static_cast<uint32_t>(getRank()) << " : Start execution\n";
    getAlgoLayer()->execute();
    if (getAlgoLayer()->isBroadcastingMessage()) {
        // Display statistics
        static std::mutex mtx;
        scoped_lock lock{mtx};
        cout << Arguments::csvHeadline() << "," << Measures::csvHeadline() << "\n";
        cout << getArguments().asCsv(getAlgoLayer()->toString(), getAlgoLayer()->getCommLayer()->toString(), to_string(getRank())) << "," << measures.asCsv()
             << "\n";
    }
    if (getArguments().getFrequency()  && getAlgoLayer()->isBroadcastingMessage())
        taskSendPeriodicPerfMessage.get();
    if (getArguments().getVerbose())
        cout << "PerfMeasures (Warning: this may not be PerfMeasures pos!) #" << static_cast<uint32_t>(getRank()) << " : End of execution\n";
}

void PerfMeasures::processFinishedPerfMeasuresMsg(rank_t senderPos)
{
    ++nbReceivedFinishedPerfMeasures;
    if (getArguments().getVerbose())
        cout << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Deliver FinishedPerfMeasures from sender pos #" << static_cast<uint32_t>(senderPos) << " (nbReceivedFinishedPerfMeasures = " << nbReceivedFinishedPerfMeasures << ")\n";

    if (nbReceivedFinishedPerfMeasures > getAlgoLayer()->getBroadcasters().size())
    {
        cerr << "ERROR : Delivering a FinishedPerfMeasures message while we already have received all FinishedPerfMeasures messages we were waiting for.\n";
        exit(EXIT_FAILURE);
    }
    if (nbReceivedFinishedPerfMeasures == getAlgoLayer()->getBroadcasters().size())
    {
        // All broadcasters are done doing measures ==> We can ask the getAlgoLayer() to terminate.
        getAlgoLayer() -> terminate();
    }
}

void PerfMeasures::processFirstBroadcastMsg(rank_t senderPos) {
    ++nbReceivedFirstBroadcast;
    if (getArguments().getVerbose())
        cout << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Deliver FirstBroadcast from sender pos #" << static_cast<uint32_t>(senderPos) << " (nbReceivedFirstBroadcast = " << nbReceivedFirstBroadcast << ")\n";
    if (nbReceivedFirstBroadcast > getAlgoLayer()->getBroadcasters().size())
    {
        cerr << "ERROR : Delivering a FirstBroadcast message while we already have received all FirstBroadcast messages we were waiting for.\n";
        exit(EXIT_FAILURE);
    }
    if (nbReceivedFirstBroadcast == getAlgoLayer()->getBroadcasters().size())
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

void PerfMeasures::processPerfMeasureMsg(rank_t senderPos, std::string && msg) {
    auto spm{deserializeStruct<SessionPerfMeasure>(std::move(msg))};
    if (getArguments().getVerbose())
        cout << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Deliver PerfMeasure from sender pos #" << static_cast<uint32_t>(senderPos) << " (senderPos = " << static_cast<uint32_t>(spm.senderPos) << " ; msgNum = " << spm.msgNum << ")\n";
    measures.addNbBytesDelivered(getArguments().getSizeMsg());
    // We check which process must send the PerfResponse. The formula hereafter guarantees that first PerfMeasure is
    // answered by successor of sender process, second PerfMeasure message is answered by successor of the successor of
    // sender process, etc.
    if ((spm.senderPos + spm.msgNum) % getAlgoLayer()->getBroadcasters().size() == getAlgoLayer()->getPosInBroadcasters())
    {
        // Current process must broadcast PerfResponse message for this PerfMeasure message.
        if (getArguments().getVerbose())
            cout << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Broadcast PerfResponse by sender pos #" << static_cast<uint32_t>(getRank()) << " (perfMeasureSenderPos = " << static_cast<uint32_t>(spm.senderPos) << " ; perfMeasureMsgNum = " << spm.msgNum << ")\n";
        auto s {serializeStruct<SessionPerfResponse>(SessionPerfResponse{SessionMsgId::PerfResponse,
                                                                         spm.senderPos,
                                                                         spm.msgNum,
                                                                         spm.sendTime,
                                                                         spm.filler})};
        getAlgoLayer()->totalOrderBroadcast(std::move(s));
    }
}

void PerfMeasures::processPerfResponseMsg(rank_t senderPos, std::string && msg) {
    auto spr{deserializeStruct<SessionPerfResponse>(std::move(msg))};
    if (getArguments().getVerbose())
        cout << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Deliver PerfResponse from sender pos #" << static_cast<uint32_t>(senderPos) << " (perfMeasureSenderPos = " << static_cast<uint32_t>(spr.perfMeasureSenderPos) << " ; perfMeasureMsgNum = " << spr.perfMeasureMsgNum << ")\n";
    measures.addNbBytesDelivered(getArguments().getSizeMsg());
    chrono::duration<double, std::milli> elapsed = std::chrono::system_clock::now() - spr.perfMeasureSendTime;
    if (spr.perfMeasureSenderPos == getAlgoLayer()->getPosInBroadcasters())
    {
        if (nbReceivedPerfResponseForSelf >= getArguments().getNbMsg()) {
            cerr << "WARNING : PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Deliver too many PerfResponse for self\n";
            return;
        }
        ++nbReceivedPerfResponseForSelf;
        if (spr.perfMeasureMsgNum >= getArguments().getNbMsg()*getArguments().getWarmupCooldown()/100/2
            && spr.perfMeasureMsgNum < getArguments().getNbMsg() - getArguments().getNbMsg()*getArguments().getWarmupCooldown()/100/2)
            measures.add(elapsed);
        if (spr.perfMeasureMsgNum == getArguments().getNbMsg() - getArguments().getNbMsg()*getArguments().getWarmupCooldown()/100/2 - 1)
            measures.setStopTime();
        if (spr.perfMeasureMsgNum < getArguments().getNbMsg() - 1)
        {
            if (!getArguments().getFrequency())
                // As we do not send periodic PerfMessage, we send another PerfMessage
                broadcastPerfMeasure();
        }
        else
        {
            // Process is done with sending PerfMeasure messages. It tells it is done to all broadcasters
            if (getArguments().getVerbose())
                cout << "PerfMeasures pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << " : Broadcast FinishedPerfMeasures by sender pos #" << static_cast<uint32_t>(getAlgoLayer()->getPosInBroadcasters()) << "\n";
            auto s {serializeStruct<SessionFinishedPerfMeasures>(SessionFinishedPerfMeasures{SessionMsgId::FinishedPerfMeasures})};
            getAlgoLayer()->totalOrderBroadcast(std::move(s));
        }
    }
}

void PerfMeasures::sendPeriodicPerfMessage() {
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
