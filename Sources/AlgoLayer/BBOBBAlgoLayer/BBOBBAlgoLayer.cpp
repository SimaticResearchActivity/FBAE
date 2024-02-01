//
// Created by lardeur on 09/10/23.
//

#include "../../SessionLayer/SessionLayer.h"
#include "BBOBBAlgoLayer.h"
#include "BBOBBAlgoLayerMsg.h"
#include "../../msgTemplates.h"
#include "../../CommLayer/TcpCommLayer/TcpCommLayer.h"

#include <algorithm>
#include <ctgmath>
#include <thread>

using namespace std;
using namespace fbae_BBOBBAlgoLayer;

/**
 * @brief Functionality similar to Java's instanceof (found at https://www.tutorialspoint.com/cplusplus-equivalent-of-instanceof)
 * @tparam Base Are we an instance of this Base type?
 * @tparam T The type we want to test
 * @param ptr A pointer on an instance of T
 * @return true if @ptr (of type @T) is an instance of type @Base.
 */
template<typename Base, typename T>
inline bool instanceof(const T *ptr) {
    return dynamic_cast<const Base*>(ptr) != nullptr;
}

bool BBOBBAlgoLayer::callbackHandleMessage(std::unique_ptr<CommPeer> peer, std::string && msgString) {
    peers_peersRank_ready.wait();
    auto msgId{static_cast<MsgId>(msgString[0])};
    switch (msgId) {
        using enum MsgId;

        case RankInfo : {
            auto bri{deserializeStruct<BroadcasterRankInfo>(std::move(msgString))};
            if (getSession()->getParam().getVerbose())
                cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank())
                     << " : Received RankInfo from broadcaster#" << static_cast<uint32_t>(bri.senderRank) << "\n";
            if (++nbConnectedBroadcasters == nbStepsInWave) {

                if (getSession()->getParam().getVerbose())
                    cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank())
                         << " : All broadcasters are connected \n";

                getSession()->callbackInitDone();

                //Send Step 0 Message of wave 0
                lastSentStepMsg.msgId = MsgId::Step;
                lastSentStepMsg.wave = -1;
                beginWave();
                CatchUpIfLateInMessageSending();
            }
            break;
        }

        case AckDisconnectIntent : {
            peer->disconnect();
            break;
        }

        case Step : {

            if (sendWave) {

                auto stepMsg{deserializeStruct<StepMsg>(std::move(msgString))};

                if (getSession()->getParam().getVerbose())
                    cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank())
                         << " : Receive a Step Message (step : " << stepMsg.step << " / wave : "
                         << stepMsg.wave << ") from Broadcaster#" << static_cast<uint32_t>(stepMsg.senderRank) << "\n";

                if (stepMsg.wave == lastSentStepMsg.wave) {
                    currentWaveReceivedStepMsg[stepMsg.step] = stepMsg;
                    CatchUpIfLateInMessageSending();
                } else if (stepMsg.wave == lastSentStepMsg.wave + 1) {
                    // Message is early and needs to be treated in the next wave
                    nextWaveReceivedStepMsg[stepMsg.step] = stepMsg;
                } else {
                    cerr << "ERROR\tBBOBBAlgoLayer/ Broadcaster #" << getSession()->getRank() << " (currentWave = " << lastSentStepMsg.wave << ") : Unexpected wave = " << stepMsg.wave
                         << " from sender #" << static_cast<int>(stepMsg.senderRank) << " (with step = " << stepMsg.step << ")\n";
                    exit(EXIT_FAILURE);
                }
            }
            break;
        }

        case DisconnectIntent : {
            auto bdi{deserializeStruct<BroadcasterDisconnectIntent>(std::move(msgString))};
            auto s{serializeStruct<StructAckDisconnectIntent>(StructAckDisconnectIntent{
                    MsgId::AckDisconnectIntent,
            })};
            peer->sendMsg(std::move(s));
            if (getSession()->getParam().getVerbose())
                cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank()) << " : Broadcaster #"
                     << static_cast<uint32_t>(bdi.senderRank) << " announced it will disconnect\n";
            break;
        }

        default: {
            cerr << "ERROR\tBBOBBAlgoLayer: Unexpected msgId (" << static_cast<int>(msgId) << ")\n";
            exit(EXIT_FAILURE);
        }
    }

    return msgId == MsgId::AckDisconnectIntent;
}

void BBOBBAlgoLayer::beginWave() {
    //Build first Step Message of a new Wave
    lastSentStepMsg.wave += 1;
    lastSentStepMsg.step = 0;
    const auto senderRank = getSession()->getRank();
    lastSentStepMsg.senderRank = senderRank;
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
             << " : Send Step Message (step : 0 / wave : " << lastSentStepMsg.wave << ") to Broadcaster #" << peersRank[lastSentStepMsg.step]
             << "\n";
    auto s{serializeStruct(lastSentStepMsg)};
    peers[lastSentStepMsg.step]->sendMsg(std::move(s));
}

void BBOBBAlgoLayer::CatchUpIfLateInMessageSending() {
    // Are we able to send new stepMsg knowing the messages received in currentWaveReceivedStepMsg?
    auto step = lastSentStepMsg.step;
    while (lastSentStepMsg.step < nbStepsInWave - 1 && currentWaveReceivedStepMsg.contains(step)) {
        // Build the new version of lastSentStepMsg
        lastSentStepMsg.step += 1;
        lastSentStepMsg.batchesBroadcast.insert(lastSentStepMsg.batchesBroadcast.end(),
                                                currentWaveReceivedStepMsg[step].batchesBroadcast.begin(), currentWaveReceivedStepMsg[step].batchesBroadcast.end() );
        // Send it
        if (getSession()->getParam().getVerbose())
            cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank())
                 << " : Send Step Message (step : " << lastSentStepMsg.step << " / wave : " << lastSentStepMsg.wave
                 << ") to Broadcaster#" << static_cast<uint32_t>(peersRank[lastSentStepMsg.step]) << "\n";
        auto s{serializeStruct(lastSentStepMsg)};
        peers[lastSentStepMsg.step]->sendMsg(std::move(s));
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
        vector<int> positions(getSession()->getParam().getSites().size(), not_found);
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
                    // because thread @SessionLayer::sendPeriodicPerfMessage may have filled up @msgsWaitingToBeBroadcast
                    shortcutBatchCtrl = true;
                    getSession()->callbackDeliver(senderRank, std::move(msg));
                    shortcutBatchCtrl = false;
                }
            }
        }

        // Prepare new wave
        currentWaveReceivedStepMsg = nextWaveReceivedStepMsg;
        nextWaveReceivedStepMsg.clear();
        beginWave();
        CatchUpIfLateInMessageSending();
    }
}

bool BBOBBAlgoLayer::executeAndProducedStatistics() {

    bool verbose = getSession()->getParam().getVerbose();

    auto commLayer = getSession()->getCommLayer();
    auto sites = getSession()->getParam().getSites();
    int rank = getSession()->getRank();

    setBroadcasters(sites);

    const auto n = static_cast<int>(getSession()->getParam().getSites().size());
    nbStepsInWave = static_cast<int>(n % 2 == 0 ? log2(n) : ceil(log2(n)));

    if (getSession()->getParam().getVerbose())
        cout << "\tBBOBBAlgoLayer / Broadcaster#" << static_cast<uint32_t>(rank) << ": Wait for connections on port " << get<PORT>(sites[rank])
             << "\n";
    commLayer->initHost(get<PORT>(sites[rank]), nbStepsInWave, this);

    std::thread t([rank, verbose, sites, this]() {
        // Send RankInfo
        for (int power_of_2 = 1; power_of_2 < sites.size(); power_of_2 *= 2) {
            std::unique_ptr<CommPeer> cp = getSession()->getCommLayer()->connectToHost(
                    getSession()->getParam().getSites()[(power_of_2 + rank) %
                                                        getSession()->getParam().getSites().size()],
                    this);
            peers.push_back(std::move(cp));
            //Data for verbose messages
            if (verbose)
                peersRank.push_back((power_of_2 + rank) % getSession()->getParam().getSites().size());
        }

        for (int power_of_2 = 1, i = 0; power_of_2 < sites.size(); power_of_2 *= 2, ++i) {
            if (verbose)
                cout << "\tBBOOBBAlgoLayer / Broadcaster#" << static_cast<uint32_t>(rank) << " : Send RankInfo to Broadcaster#"
                     << (power_of_2 + rank) % getSession()->getParam().getSites().size() << "\n";
            auto s{serializeStruct<BroadcasterRankInfo>(BroadcasterRankInfo{
                    MsgId::RankInfo,
                    getSession()->getRank()
            })};
            peers[i]->sendMsg(std::move(s));
        }
        peers_peersRank_ready.count_down();
        if (verbose)
            cout << "\tBBOOBBAlgoLayer / Broadcaster#" << static_cast<uint32_t>(rank) << " : Sent all RankInfo messages\n";
    });

    // We wait for all connectToHost to be done (and RankInfo messages sent) before we call waitForMsg
    // Note: This way of doing is only compatible with Tcp layer not Enet layer! This is because Enet layer
    //       requires to call waitForMsg in order to receive connection confirmation . Thus, we may receive
    //       connection confirmations and, at the same time, protocol messages. As this is hard to reason, we
    //       only authorize Tcp layer.
    if (!instanceof<TcpCommLayer>(getSession()->getCommLayer())) {
        cerr << "ERROR : " << toString() << " is only compatible with Tcp communication layer, but you specified "
             << getSession()->getCommLayer()->toString() << " communication layer in program arguments "
             << "==> Specify Tcp communication layer in program arguments.\n";
        exit (EXIT_FAILURE);
    }
    t.join();

    if (verbose)
        cout << "\tBBOOBBAlgoLayer / Broadcaster#" << static_cast<uint32_t>(rank) << " : Wait for messages\n";
    commLayer->waitForMsg(nbStepsInWave);

    if (verbose)
        cout << "Broadcaster #" << static_cast<uint32_t>(rank)
             << " Finished waiting for messages ==> Giving back control to SessionLayer\n";

    return true;
}

void BBOBBAlgoLayer::terminate() {
    sendWave = false;
    auto s{serializeStruct<BroadcasterDisconnectIntent>(BroadcasterDisconnectIntent{
            MsgId::DisconnectIntent,
            getSession()->getRank(),
    })};
    for (auto i = 0; i < peers.size(); ++i) {
        if (getSession()->getParam().getVerbose())
            cout << "\tBBOOBBAlgoLayer / Broadcaster #" << static_cast<uint32_t>(getSession()->getRank()) << " announces to broadcaster #" << static_cast<uint32_t>(peersRank[i]) << " it will disconnect\n";
        peers[i]->sendMsg(std::move(s));
    }

}

std::string BBOBBAlgoLayer::toString() {
    return "BBOBB";
}

void BBOBBAlgoLayer::totalOrderBroadcast(std::string && msg) {
    unique_lock lck(mtxBatchCtrl);
    condVarBatchCtrl.wait(lck, [this] {
        return (msgsWaitingToBeBroadcast.size() * getSession()->getParam().getSizeMsg() < getSession()->getParam().getMaxBatchSize())
               || shortcutBatchCtrl;
    });
    msgsWaitingToBeBroadcast.emplace_back(std::move(msg));
}
