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

void BBOBB::callbackHandleMessage(std::string && msgString) {
    auto msgId{static_cast<MsgId>(msgString[0])};
    if (msgId == MsgId::Step) {
        if (!algoTerminated) {
            auto stepMsg{deserializeStruct<StepMsg>(std::move(msgString))};

            if (getSession()->getParam().getVerbose())
                cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank())
                     << " : Receive a Step Message (wave : " << stepMsg.wave << " / step : "
                     << stepMsg.step << ") from Broadcaster #" << static_cast<uint32_t>(stepMsg.senderRank) << "\n";

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
                cerr << "\tERROR\tBBOBBAlgoLayer/ Broadcaster #" << static_cast<uint32_t>(getSession()->getRank())
                     << " (currentWave = " << lastSentStepMsg.wave << ") : Unexpected wave = " << stepMsg.wave
                     << " (with step = " << stepMsg.step << ") from Broadcaster #"
                     << static_cast<uint32_t>(stepMsg.senderRank) << "\n";
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
    const auto senderRank = getSession()->getRank();
    lastSentStepMsg.senderRank = senderRank;
    lastSentStepMsg.wave += 1;
    lastSentStepMsg.step = 0;
    lastSentStepMsg.batchesBroadcast.clear();
    {
        lock_guard lck(mtxBatchCtrl);
        BatchSessionMsg newMessage{senderRank, std::move(msgsWaitingToBeBroadcast)};
        msgsWaitingToBeBroadcast.clear();
        lastSentStepMsg.batchesBroadcast.emplace_back(std::move(newMessage));
    }
    condVarBatchCtrl.notify_one();

    // Send it
    if (getSession()->getParam().getVerbose())
        cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank())
             << " : Send Step Message (wave : " << lastSentStepMsg.wave << " / step : 0) to Broadcaster #" << static_cast<uint32_t>(peersRank[lastSentStepMsg.step])
             << "\n";
    getSession()->getCommLayer()->send(peersRank[lastSentStepMsg.step],
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
        if (getSession()->getParam().getVerbose())
            cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank())
                 << " : Send Step Message (wave : " << lastSentStepMsg.wave << " / step : " << lastSentStepMsg.step
                 << ") to Broadcaster #" << static_cast<uint32_t>(peersRank[lastSentStepMsg.step]) << "\n";
        getSession()->getCommLayer()->send(peersRank[lastSentStepMsg.step],
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
        vector<int> positions(getBroadcastersRank().size(), not_found);
        for (int pos = 0 ; pos < batches.size() ; ++pos) {
            positions[batches[pos].senderRank] = pos;
        }

        // Deliver the different BatchSessionMsg
        for (auto const& pos: positions) {
            if (pos != not_found) {
                auto senderRank = batches[pos].senderRank;
                for (auto & msg : batches[pos].batchSessionMsg) {
                    // We surround the call to @callbackDeliver method with shortcutBatchCtrl = true; and
                    // shortcutBatchCtrl = false; This is because callbackDeliver() may lead to a call to
                    // @totalOrderBroadcast method which could get stuck in condVarBatchCtrl.wait() instruction
                    // because task @SessionLayer::sendPeriodicPerfMessage may have filled up @msgsWaitingToBeBroadcast
                    shortcutBatchCtrl = true;
                    getSession()->callbackDeliver(senderRank, std::move(msg));
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
    // Compute vector of broadcasters rank
    vector<rank_t> v(getSession()->getParam().getSites().size()); // All participants are broadcasting.
    std::iota(v.begin(), v.end(), 0); // @broadcastersRank must always start with 0, if we want @Session::processPerfMeasureMsg() to work properly.
    setBroadcastersRank(std::move(v));

    // Prepare call to @CommLayer::openDestAndWaitIncomingMsg()
    const int rank = getSession()->getRank();
    vector<rank_t> dest;
    for (int power_of_2 = 1; power_of_2 < getBroadcastersRank().size(); power_of_2 *= 2) {
        auto rankOutgoingPeer = static_cast<rank_t>((rank + power_of_2) % getBroadcastersRank().size());
        dest.push_back(rankOutgoingPeer);
        peersRank.push_back(rankOutgoingPeer);
        ++nbStepsInWave;
    }
    getSession()->getCommLayer()->openDestAndWaitIncomingMsg(dest, nbStepsInWave, this);

    if (getSession()->getParam().getVerbose())
        cout << "Broadcaster #" << static_cast<uint32_t>(rank)
             << " Finished waiting for messages ==> Giving back control to SessionLayer\n";
}

void BBOBB::terminate() {
    algoTerminated = true;
    getSession()->getCommLayer()->terminate();
}

std::string BBOBB::toString() {
    return "BBOBB";
}

void BBOBB::totalOrderBroadcast(std::string && msg) {
    unique_lock lck(mtxBatchCtrl);
    condVarBatchCtrl.wait(lck, [this] {
        return (msgsWaitingToBeBroadcast.size() * getSession()->getParam().getSizeMsg() < getSession()->getParam().getMaxBatchSize())
               || shortcutBatchCtrl;
    });
    msgsWaitingToBeBroadcast.emplace_back(std::move(msg));
}
