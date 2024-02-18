//
// Created by lardeur on 09/10/23.
//

#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "BBOBB.h"
#include "BBOBBMsg.h"
#include "../../msgTemplates.h"

#include <algorithm>

using namespace std;
using namespace fbae_BBOBBAlgoLayer;

BBOBB::BBOBB(std::unique_ptr<CommLayer> commLayer)
        : AlgoLayer{std::move(commLayer)}
{
}

void BBOBB::callbackReceive(std::string && msgString) {
    auto msgId{static_cast<MsgId>(msgString[0])};
    if (msgId == MsgId::Step) {
        if (!algoTerminated) {
            auto stepMsg{deserializeStruct<StepMsg>(std::move(msgString))};

            if (getSessionLayer()->getArguments().getVerbose())
                cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getPosInBroadcastersGroup().value())
                     << " : Receive a Step Message (wave : " << stepMsg.wave << " / step : "
                     << stepMsg.step << ") from Broadcaster #" << static_cast<uint32_t>(stepMsg.senderPos) << "\n";

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
                cerr << "\tERROR\tBBOBBAlgoLayer/ Broadcaster #" << static_cast<uint32_t>(getPosInBroadcastersGroup().value())
                     << " (currentWave = " << lastSentStepMsg.wave << ") : Unexpected wave = " << stepMsg.wave
                     << " (with step = " << stepMsg.step << ") from Broadcaster #"
                     << static_cast<uint32_t>(stepMsg.senderPos) << "\n";
                exit(EXIT_FAILURE);
            }
        }
    } else {
        cerr << "ERROR\tBBOBBAlgoLayer: Unexpected msgId (" << static_cast<int>(msgId) << ")\n";
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
    {
        lock_guard lck(mtxBatchCtrl);
        BatchSessionMsg newMessage{senderPos, std::move(msgsWaitingToBeBroadcast)};
        msgsWaitingToBeBroadcast.clear();
        lastSentStepMsg.batchesBroadcast.emplace_back(std::move(newMessage));
    }
    condVarBatchCtrl.notify_one();

    // Send it
    if (getSessionLayer()->getArguments().getVerbose())
        cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getPosInBroadcastersGroup().value())
             << " : Send Step Message (wave : " << lastSentStepMsg.wave << " / step : 0) to Broadcaster #" << static_cast<uint32_t>(peersPos[lastSentStepMsg.step])
             << "\n";
    getCommLayer()->send(peersPos[lastSentStepMsg.step],
                                            serializeStruct(lastSentStepMsg));
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
        if (getSessionLayer()->getArguments().getVerbose())
            cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getPosInBroadcastersGroup().value())
                 << " : Send Step Message (wave : " << lastSentStepMsg.wave << " / step : " << lastSentStepMsg.step
                 << ") to Broadcaster #" << static_cast<uint32_t>(peersPos[lastSentStepMsg.step]) << "\n";
        getCommLayer()->send(peersPos[lastSentStepMsg.step],
                                                serializeStruct(lastSentStepMsg));
        step = lastSentStepMsg.step;
    }
    //After all the messages of one wave have been received (and thus sent), deliver the messages of this wave.
    if (currentWaveReceivedStepMsg.size() == nbStepsInWave) {
        // Build vector of BatchSessionMsg to deliver
        // Note that to append currentWaveReceivedStepMsg[nbStepsInWave - 1].batchesBroadcast to batches, we
        // do not use batches.insert() but a for loop in order to be able to use std::move()
        vector<BatchSessionMsg> batches{std::move(lastSentStepMsg.batchesBroadcast)};
        for (auto & batch : currentWaveReceivedStepMsg[nbStepsInWave - 1].batchesBroadcast)
            batches.push_back(std::move(batch));

        // Compute the position in batches vector of participant rank 0, then participant rank 1, etc.
        // For example, positions[0] represents position of BatchSessionMsg of participant 0 in batches vector.
        // Note: If BatchSessionMsg of a participant appears twice, we memorize only one position
        //       (thus, afterward, we will not deliver twice this BatchSessionMsg).
        constexpr int not_found = -1;
        vector<int> positions(getBroadcastersGroup().size(), not_found);
        for (int pos = 0 ; pos < batches.size() ; ++pos) {
            positions[batches[pos].senderPos] = pos;
        }

        // Deliver the different BatchSessionMsg
        for (auto const& pos: positions) {
            if (pos != not_found) {
                auto senderRank = batches[pos].senderPos;
                for (auto & msg : batches[pos].batchSessionMsg) {
                    // We surround the call to @callbackDeliver method with shortcutBatchCtrl = true; and
                    // shortcutBatchCtrl = false; This is because callbackDeliver() may lead to a call to
                    // @totalOrderBroadcast method which could get stuck in condVarBatchCtrl.wait() instruction
                    // because task @SessionLayer::sendPeriodicPerfMessage may have filled up @msgsWaitingToBeBroadcast
                    shortcutBatchCtrl = true;
                    getSessionLayer()->callbackDeliver(senderRank, std::move(msg));
                    shortcutBatchCtrl = false;
                    if (algoTerminated) {
                        return;
                    }
                }
            }
        }

        // Prepare new wave
        currentWaveReceivedStepMsg = nextWaveReceivedStepMsg;
        nextWaveReceivedStepMsg.clear();
        beginWave();
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

    if (getSessionLayer()->getArguments().getVerbose())
        cout << "Broadcaster #" << static_cast<uint32_t>(pos)
             << " Finished waiting for messages ==> Giving back control to SessionLayer\n";
}

void BBOBB::terminate() {
    algoTerminated = true;
    getCommLayer()->terminate();
}

std::string BBOBB::toString() {
    return "BBOBB";
}

void BBOBB::totalOrderBroadcast(std::string && msg) {
    unique_lock lck(mtxBatchCtrl);
    condVarBatchCtrl.wait(lck, [this] {
        return (msgsWaitingToBeBroadcast.size() * getSessionLayer()->getArguments().getSizeMsg() < getSessionLayer()->getArguments().getMaxBatchSize())
               || shortcutBatchCtrl;
    });
    msgsWaitingToBeBroadcast.emplace_back(std::move(msg));
}
