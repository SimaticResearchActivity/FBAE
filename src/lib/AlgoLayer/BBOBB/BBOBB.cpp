//
// Created by lardeur on 09/10/23.
//

#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "../AlgoLayerMsg.h"
#include "BBOBB.h"
#include "BBOBBMsg.h"
#include "../../msgTemplates.h"

#include <algorithm>

using namespace std;
using namespace fbae_BBOBBAlgoLayer;

BBOBB::BBOBB(std::unique_ptr<CommLayer> commLayer)
        : AlgoLayer{std::move(commLayer), "fbae.algo.BBOBB"}
{
}

void BBOBB::callbackReceive(std::string && algoMsgAsString) {
    auto msgId{static_cast<MsgId>(algoMsgAsString[0])};
    if (msgId == MsgId::Step) {
        if (!algoTerminated) {
            processStepMsg(std::move(algoMsgAsString));
        }
    } else {
        LOG4CXX_ERROR_FMT(getAlgoLogger(), "Unexpected msgId ({:d})", static_cast<uint32_t>(msgId));
        exit(EXIT_FAILURE);
   }
}

void BBOBB::callbackInitDone() {
    AlgoLayer::callbackInitDone();
    lastSentStepMsg.wave = -1;
    beginWave();
    // As @CommLayer guarantees that we do not receive any message before @BBOBB::callbackInitDone() is finished,
    // we o not need to call catchUpIfLateInMessageSending()
}

void BBOBB::beginWave() {
    //Build first Step Message of a new Wave
    lastSentStepMsg.msgId = MsgId::Step;
    const auto senderPos = getPosInBroadcastersGroup().value();
    lastSentStepMsg.senderPos = senderPos;
    lastSentStepMsg.wave += 1;
    lastSentStepMsg.step = 0;
    lastSentStepMsg.batchesBroadcast.clear();
    if (auto batch{batchGetBatchMsgs(senderPos)} ; batch.has_value()) {
        lastSentStepMsg.batchesBroadcast.emplace_back(batch.value());
    }

    // Send it
    LOG4CXX_INFO_FMT(getAlgoLogger(), "Broadcaster #{:d} : Send Step Message (wave : {:d} / step : 0) to Broadcaster #", getPosInBroadcastersGroup().value(), lastSentStepMsg.wave, peersPos[lastSentStepMsg.step]);
    
    getCommLayer()->send(peersPos[lastSentStepMsg.step], serializeStruct(lastSentStepMsg));
}

void BBOBB::catchUpIfLateInMessageSending() {
    // Are we able to send new stepMsg knowing the messages received in currentWaveReceivedStepMsg?
    auto step = lastSentStepMsg.step;
    while (step < nbStepsInWave - 1 && currentWaveReceivedStepMsg.contains(step)) {
        // Build the new version of lastSentStepMsg
        lastSentStepMsg.step += 1;
        lastSentStepMsg.batchesBroadcast.insert(lastSentStepMsg.batchesBroadcast.end(),
                                                currentWaveReceivedStepMsg[step].batchesBroadcast.begin(), currentWaveReceivedStepMsg[step].batchesBroadcast.end() );
        // Send it
        LOG4CXX_INFO_FMT(getAlgoLogger(), "Broadcaster #{:d} : Send Step Message (wave : {:d} / step : 0) to Broadcaster #", getPosInBroadcastersGroup().value(), lastSentStepMsg.wave, lastSentStepMsg.step, peersPos[lastSentStepMsg.step]);

        getCommLayer()->send(peersPos[lastSentStepMsg.step], serializeStruct(lastSentStepMsg));
        step = lastSentStepMsg.step;
    }
    //After all the messages of one wave have been received (and thus sent), deliver the messages of this wave.
    if (currentWaveReceivedStepMsg.size() == nbStepsInWave) {
        deliverBatchSessionMsg();
        if (algoTerminated)
            return;

        // Prepare new wave
        currentWaveReceivedStepMsg = nextWaveReceivedStepMsg;
        nextWaveReceivedStepMsg.clear();
        beginWave();
    }
}

void BBOBB::deliverBatchSessionMsg() {
    // Build vector of BatchSessionMsg to deliver
    // Note that to append currentWaveReceivedStepMsg[nbStepsInWave - 1].batchesBroadcast to batches, we
    // do not use batches.insert() but a for loop in order to be able to use std::move()
    vector<fbae_AlgoLayer::BatchSessionMsg> batches{std::move(lastSentStepMsg.batchesBroadcast)};
    for (auto & batch : currentWaveReceivedStepMsg[nbStepsInWave - 1].batchesBroadcast)
        batches.push_back(std::move(batch));

    // Compute the position in batches vector of participant rank 0, then participant rank 1, etc.
    // For example, positions[0] represents position of BatchSessionMsg of participant 0 in batches vector.
    // Note: If BatchSessionMsg of a participant appears twice, we memorize only one position
    //       (thus, afterward, we will not deliver twice this BatchSessionMsg).
    map<rank_t, size_t> positions;
    for (int pos = 0 ; pos < batches.size() ; ++pos) {
        positions[batches[pos].senderPos] = pos;
    }

    // Deliver the different BatchSessionMsg
    for (auto const& [senderRank, posInBatches]: positions) {
        for (auto const& msg : batches[posInBatches].batchSessionMsg) {
            batchNoDeadlockCallbackDeliver(senderRank, msg);
            if (algoTerminated) {
                return;
            }
        }
    }
}

void BBOBB::execute() {
    // Compute vector of broadcasters pos
    vector<rank_t> v(getSessionLayer()->getArguments().getSites().size()); // All participants are broadcasting.
    std::iota(v.begin(), v.end(), 0);
    setBroadcastersGroup(std::move(v));

    // Prepare call to @CommLayer::openDestAndWaitIncomingMsg()
    const auto pos = getPosInBroadcastersGroup().value();
    vector<rank_t> dest;
    for (int power_of_2 = 1; power_of_2 < getBroadcastersGroup().size(); power_of_2 *= 2) {
        auto rankOutgoingPeer = static_cast<rank_t>((pos + power_of_2) % getBroadcastersGroup().size());
        dest.push_back(rankOutgoingPeer);
        peersPos.push_back(rankOutgoingPeer);
        ++nbStepsInWave;
    }
    getCommLayer()->openDestAndWaitIncomingMsg(dest, nbStepsInWave, this);

    LOG4CXX_INFO_FMT(getAlgoLogger(), "Broadcaster #{:d} Finished waiting for messages ==> Giving back control to SessionLayer", pos);
}

void BBOBB::processStepMsg(string &&algoMsgAsString) {
    auto stepMsg{deserializeStruct<StepMsg>(std::move(algoMsgAsString))};

    LOG4CXX_INFO_FMT(getAlgoLogger(), "Broadcaster #{:d} : Receive a Step Message (wave : {:d} / step : {:d}) from Broadcaster #{:d}", getPosInBroadcastersGroup().value(), stepMsg.wave, stepMsg.step, stepMsg.senderPos);

    if (stepMsg.wave == lastSentStepMsg.wave) {
        currentWaveReceivedStepMsg[stepMsg.step] = stepMsg;
        catchUpIfLateInMessageSending();
        if (!algoTerminated) {
            catchUpIfLateInMessageSending();
        }
    } else if (stepMsg.wave == lastSentStepMsg.wave + 1) {
        // Message is early and needs to be treated in the next wave
        nextWaveReceivedStepMsg[stepMsg.step] = stepMsg;
    } else {
        LOG4CXX_ERROR_FMT(getAlgoLogger(), "Broadcaster #{:d} (currentWave = {:d}) : Unexpected wave = {:d} (with step = {:d}) from Broadcaster #{:d}" , getPosInBroadcastersGroup().value(), lastSentStepMsg.wave, stepMsg.wave, stepMsg.step, stepMsg.senderPos);
        exit(EXIT_FAILURE);
    }
}

void BBOBB::terminate() {
    algoTerminated = true;
    getCommLayer()->terminate();
}

std::string BBOBB::toString() {
    return "BBOBB";
}

