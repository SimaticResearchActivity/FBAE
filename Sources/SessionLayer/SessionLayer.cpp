#include <iostream>
#include <mutex>
#include "SessionLayer.h"
#include "SessionLayerMsg.h"
#include "../msgTemplates.h"

using namespace std;
using namespace fbae_SessionLayer;

SessionLayer::SessionLayer(const Param &param, int rank, std::unique_ptr<AlgoLayer> algoLayer, std::unique_ptr<CommLayer> commLayer)
: param{param}
, rank{rank}
, algoLayer{std::move(algoLayer)}
, commLayer{std::move(commLayer)}
, measures{static_cast<size_t>(param.getNbMsg())}
{
    this->algoLayer->setSession(this);
}

unsigned int SessionLayer::broadcastPerfMeasure(unsigned int msgNum) {
    ++msgNum;
    if (param.getVerbose())
        cout << "SessionLayer #" << rank << " : Broadcast PerfMeasure by sender #" << rank << " (msgNum = " << msgNum << ")\n";
    auto s {serializeStruct<SessionPerfMeasure>(SessionPerfMeasure{SessionMsgId::PerfMeasure,
                                                                   static_cast<unsigned char>(rank),
                                                                   msgNum,
                                                                   std::chrono::system_clock::now(),
                                                                   std::string(
                                                                           param.getSizeMsg() -
                                                                           minSizeClientMessageToBroadcast,
                                                                           0)})};
    algoLayer->totalOrderBroadcast(s);
    return msgNum;
}

void SessionLayer::callbackDeliver(int senderRank, int seqNum, const std::string &msg) {
    thread_local unsigned int msgNum = 0;
    switch (SessionMsgId sessionMsgTyp{ static_cast<SessionMsgId>(msg[0]) }; sessionMsgTyp)
    {
        using enum SessionMsgId;
        case FinishedPerfMeasures :
            processFinishedPerfMeasuresMsg(senderRank, seqNum);
            break;
        case FirstBroadcast :
            msgNum = processFirstBroadcastMsg(senderRank, seqNum, msgNum);
            break;
        case PerfMeasure :
            processPerfMeasureMsg(senderRank, seqNum, msg);
            break;
        case PerfResponse :
            msgNum = processPerfResponseMsg(senderRank, seqNum, msgNum, msg);
            break;
        default:
        {
            cerr << "ERROR: Unexpected sessionMsgTyp (" << static_cast<int>(sessionMsgTyp) << ")\n";
            exit(EXIT_FAILURE);
        }
    }
}

void SessionLayer::callbackInitDone() const
{
    // Broadcast FirstBroadcast
    if (param.getVerbose())
        cout << "SessionLayer #" << rank << " : Broadcast FirstBroadcast by sender #" << rank << "\n";
    auto s {serializeStruct<SessionFirstBroadcast>(SessionFirstBroadcast{SessionMsgId::FirstBroadcast})};
    algoLayer->totalOrderBroadcast(s);
}

void SessionLayer::execute()
{
    if (param.getVerbose())
        cout << "SessionLayer #" << rank << " : Start execution\n";
    if (algoLayer->executeAndProducedStatistics())
    {
        // Display statistics
        static std::mutex mtx;
        {
            scoped_lock lock{mtx};
            cout << Param::csvHeadline() << "," << Measures::csvHeadline() << "\n";
            cout << param.asCsv(algoLayer->toString(), commLayer->toString()) << "," << measures.asCsv(param.getNbMsg(), algoLayer->getBroadcasters().size()) << "\n";
        }
    }
    if (param.getVerbose())
        cout << "SessionLayer #" << rank << " : End of execution\n";
}

CommLayer *SessionLayer::getCommLayer() const
{
    return commLayer.get();
}

const Param &SessionLayer::getParam() const
{
    return param;
}

int SessionLayer::getRank() const {
    return rank;
}

void SessionLayer::processFinishedPerfMeasuresMsg(int senderRank, int seqNum)
{
    thread_local size_t nbFinishedPerfMeasures = algoLayer->getBroadcasters().size();
    if (param.getVerbose())
        cout << "SessionLayer #" << rank << " : Deliver FinishedPerfMeasures from sender #" << senderRank << " (seqNum = " << seqNum << ")\n";
    cout << "NB Finished PErf Measure : " << nbFinishedPerfMeasures << "\n";

    if (nbFinishedPerfMeasures <= 0)
    {
        cerr << "ERROR : Delivering a FinishedPerfMeasures message while we already have received all FinishedPerfMeasures messages we were waiting for.\n";
        exit(EXIT_FAILURE);
    }
    if (--nbFinishedPerfMeasures == 0)
    {
        // All broadcasters are done doing measures ==> We can ask the AlgoLayer to terminate.
        algoLayer -> terminate();
    }
}

unsigned int SessionLayer::processFirstBroadcastMsg(int senderRank, int seqNum, unsigned int msgNum) {
    thread_local size_t nbFirstBroadcastToReceive = algoLayer->getBroadcasters().size();

    if (param.getVerbose())
        cout << "SessionLayer #" << rank << " : Deliver FirstBroadcast from sender #" << senderRank << " (seqNum = " << seqNum << ")\n";
    if (nbFirstBroadcastToReceive <= 0)
    {
        cerr << "ERROR : Delivering a FirstBroadcast message while we already have received all FirstBroadcast messages we were waiting for.\n";
        exit(EXIT_FAILURE);
    }
    if (--nbFirstBroadcastToReceive == 0)
    {
        // As we have received all awaited FirstBroadcast messages, we know that @AlgoLayer is fully
        // operational ==> We can start our performance measures ==> We broadcast a PerfMeasure message.
        msgNum = broadcastPerfMeasure(msgNum);
    }
    return msgNum;
}

void SessionLayer::processPerfMeasureMsg(int senderRank, int seqNum, const std::string &msg) {
    if (param.getVerbose())
        cout << "SessionLayer #" << rank << " : Deliver PerfMeasure from sender #" << senderRank << " (seqNum = " << seqNum << ")\n";
    auto spm{deserializeStruct<SessionPerfMeasure>(msg)};
    // We check which process must send the PerfResponse. The formula hereafter guarantees that first PerfMeasure is
    // answered by successor of sender process, second PerfMeasure message is answered by successor of successor of
    // sender process, etc.
    if ((senderRank + spm.msgNum) % algoLayer->getBroadcasters().size() == rank)
    {
        // Current process must broadcast PerfResponse message for this PerfMeasure message.
        if (param.getVerbose())
            cout << "SessionLayer #" << rank << " : Broadcast PerfResponse by sender #" << rank << "\n";
        auto s {serializeStruct<SessionPerfResponse>(SessionPerfResponse{SessionMsgId::PerfResponse,
                                                                         spm.senderRank,
                                                                         spm.msgNum,
                                                                         spm.sendTime,
                                                                         spm.filler})};
        algoLayer->totalOrderBroadcast(s);
    }
}

unsigned SessionLayer::processPerfResponseMsg(int senderRank, int seqNum, unsigned int msgNum, const string &msg) {
    if (param.getVerbose())
        cout << "SessionLayer #" << rank << " : Deliver PerfResponse from sender #" << senderRank << " (seqNum = " << seqNum << ")\n";
    auto spr{deserializeStruct<SessionPerfResponse>(msg)};
    chrono::duration<double, std::milli> elapsed = std::chrono::system_clock::now() - spr.perfMeasureSendTime;
    cout << (int)spr.perfMeasureSenderRank << " INFO " << rank << "\n";
    if (spr.perfMeasureSenderRank == rank)
    {
        measures.add(elapsed);
        cout << "PerfResponse :  " << spr.perfMeasureMsgNum  << " / " << param.getNbMsg() << "\n";
        if (spr.perfMeasureMsgNum < param.getNbMsg())
        {
            // We send another PerfMessage
            msgNum = broadcastPerfMeasure(msgNum);
        }
        else
        {
            // Process is done with Perf measures. It tells it to all broadcasters
            if (param.getVerbose())
                cout << "SessionLayer #" << rank << " : Broadcast FinishedPerfMeasures by sender #" << rank << "\n\n\n";
            auto s {serializeStruct<SessionFinishedPerfMeasures>(SessionFinishedPerfMeasures{SessionMsgId::FinishedPerfMeasures})};
            algoLayer->totalOrderBroadcast(s);
        }
    }
    return msgNum;
}
