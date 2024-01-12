//
// Created by lardeur on 09/10/23.
//

#include "../../SessionLayer/SessionLayer.h"
#include "BBOBBAlgoLayer.h"
#include "BBOBBAlgoLayerMsg.h"
#include "../../msgTemplates.h"

#include <tgmath.h>
#include <unistd.h>
#include <thread>

using namespace std;
using namespace fbae_BBOBBAlgoLayer;


bool BBOBBAlgoLayer::callbackHandleMessage(std::unique_ptr<CommPeer> peer, const std::string &msgString) {
    static atomic_int32_t seqNum{0};
    static atomic_int32_t nbConnectedBroadcasters{0};
    MsgId msgId{static_cast<MsgId>(msgString[0])};
    switch (msgId) {
        using
        enum MsgId;

        case RankInfo : {
            auto bri{deserializeStruct<BroadcasterRankInfo>(msgString)};
            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank()
                     << " : received RankInfo from broadcaster#" << static_cast<unsigned int>(bri.senderRank) << "\n";

            atomic_int32_t nbConnectedBroadcastersWanted;
            int n = getSession()->getParam().getSites().size();
            if (n % 2 == 0) {
                nbConnectedBroadcastersWanted = log2(n);
            } else {
                nbConnectedBroadcastersWanted = floor(log2(n)) + 1;
            }

            if (++nbConnectedBroadcasters == nbConnectedBroadcastersWanted) {

                if (getSession()->getParam().getVerbose())
                    cout << "Broadcaster #" << getSession()->getRank() << " all broadcasters are connected \n";

                //Necessary so that every participant received all of its rank info before the others messages
                sleep(2);

                messagesOfOneWave.resize(peers.size() + 1);
                messagesOfNextWave.resize(peers.size() + 1);
                alreadySent.resize(peers.size());
                alreadySentNextWave.resize(peers.size());

                for (int i = 0; i < peers.size(); i++) {
                    alreadySent[i] = false;
                    alreadySentNextWave[i] = false;
                }
                received.resize(peers.size() + 1);
                receivedNextWave.resize(peers.size() + 1);
                for (int i = 0; i < peers.size() + 1; i++) {
                    received[i] = false;
                    receivedNextWave[i] = false;
                }
                getSession()->callbackInitDone();

                //Send Step 0 Message of wave 0
                if (getSession()->getParam().getVerbose())
                    cout << "Broadcaster #" << getSession()->getRank()
                         << " send step 0 of wave 0 to the first member of peers " << peers[0] << "\n";

                auto s{serializeStruct(StepMessage{
                        Step,
                        static_cast<unsigned char>(getSession()->getRank()),
                        0,
                        0,
                        msgWaitingToBeBroadcasted
                })};
                peers[0]->sendMsg(s);
                msgWaitingToBeBroadcasted.clear();
            }
            break;
        }

        case AckDisconnectIntent :
            peer->disconnect();
            break;

        case Step : {
            auto bmtb{deserializeStruct<StepMessage>(msgString)};

            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank()
                     << " received a step message from broadcater " << static_cast<unsigned int>(bmtb.senderRank)
                     << " of step " << bmtb.stepNumber << " and wave " << bmtb.wave << "\n";

            if (currentWave == bmtb.wave && /* TODO delete when not needeed anymore */ currentWave < 100) {
                messagesOfOneWave[bmtb.stepNumber + 1] = bmtb.msgBroadcasted;
                received[bmtb.stepNumber + 1] = true;

                bool empty = false;

                for (int i = 1; i < messagesOfOneWave.size() - 1; i++) {

                    if (received[i]) {
                        if (!alreadySent[i]) {

                            std::vector<std::string> messageToSend;
                            for (int j = 0; j < i + 1; j++) {
                                messageToSend.insert(messageToSend.end(), messagesOfOneWave[j].begin(),
                                                     messagesOfOneWave[j].end());
                            }
                            auto mes{serializeStruct(StepMessage{
                                    Step,
                                    static_cast<unsigned char>(getSession()->getRank()),
                                    i,
                                    currentWave,
                                    messageToSend
                            })};

                            if (getSession()->getParam().getVerbose())
                                cout << "Broadcaster #" << getSession()->getRank() << " sends message of step number"
                                     << i << " from wave " << currentWave << " to broadcater"
                                     << /* //TODO Not right it is pow2... */ i << "\n";;
                            peers[i + 1]->sendMsg(mes);
                            alreadySent[i] = true;
                        }
                    } else {
                        empty = true;
                        break;
                    }
                }


                //After all the messages of one wave have been sent we deliver the messages
                if (!empty && received.back()) {

                    //We have to copy messageOfOneWave so that before delivering the messages we can clear messageOfOneWave enabling it to receive the new messages that will arrive
                    std::vector<std::vector<std::string>> messagesOfOneWaveCOPY = messagesOfOneWave;

                    //Clear the vectors we used
                    messagesOfOneWave.clear();
                    messagesOfOneWave.resize(peers.size() + 1);
                    for (int i = 0; i < peers.size(); i++) {
                        if (!messagesOfNextWave[i].empty()) {
                            messagesOfOneWave[i] = messagesOfNextWave[i];
                        }
                    }
                    messagesOfNextWave.clear();
                    messagesOfNextWave.resize(peers.size() + 1);
                    for (int i = 0; i < peers.size(); i++) {
                        alreadySent[i] = false;
                        alreadySent[i] = alreadySentNextWave[i];
                        alreadySentNextWave[i] = false;
                    }
                    for (int i = 0; i < peers.size() + 1; i++) {
                        received[i] = false;
                        received[i] = receivedNextWave[i];
                        receivedNextWave[i] = false;
                    }

                    //The n-1 messages have to be delivered
                    for (int i = 0; i < messagesOfOneWaveCOPY.size() - 1; i++) {
                        for (std::string &message: messagesOfOneWaveCOPY[i]) {
                            auto s{deserializeStruct<Message>(message)};
                            getSession()->callbackDeliver(s.senderRank, s.seqNum, s.sessionMsg);
                        }
                    }

                    //The nth message might have duplicates
                    //Function to get the number of messages we have to deliver
                    int numberOfDelivers;
                    int n = getSession()->getParam().getSites().size();
                    if (n % 2 == 0) {
                        numberOfDelivers = log2(n);
                    } else {
                        numberOfDelivers = n - pow(2, floor(log2(n)));
                    }
                    for (int i = 0; i < numberOfDelivers; i++) {
                        if (!messagesOfOneWaveCOPY.back().empty()) {
                            if (messagesOfOneWaveCOPY.back().size() > i) {
                                auto s{deserializeStruct<Message>(messagesOfOneWaveCOPY.back()[i])};
                                getSession()->callbackDeliver(s.senderRank, s.seqNum, s.sessionMsg);
                            }
                        }
                    }

                    currentWave++;
                    //All messages were delivered so we can launch another wave
                    auto s{serializeStruct(StepMessage{
                            Step,
                            static_cast<unsigned char>(getSession()->getRank()),
                            0,
                            currentWave,
                            msgWaitingToBeBroadcasted
                    })};
                    cout << "Broadcaster #" << getSession()->getRank()
                         << " send step 0 of wave " << currentWave << " to the first member of peers " << peers[0]
                         << "\n";;
                    peers[0]->sendMsg(s);
                    msgWaitingToBeBroadcasted.clear();

                }
            } else {

                //Message is early and needs to be treated in the next wave
                messagesOfNextWave[bmtb.stepNumber + 1] = bmtb.msgBroadcasted;
                receivedNextWave[bmtb.stepNumber + 1] = true;

                for (int i = 1; i < messagesOfNextWave.size() - 1; i++) {

                    if (receivedNextWave[i]) {
                        if (!alreadySentNextWave[i]) {

                            std::vector<std::string> messageToSend;
                            for (int j = 0; j < i + 1; j++) {
                                messageToSend.insert(messageToSend.end(), messagesOfNextWave[j].begin(),
                                                     messagesOfNextWave[j].end());
                            }
                            auto mes{serializeStruct(StepMessage{
                                    Step,
                                    static_cast<unsigned char>(getSession()->getRank()),
                                    i,
                                    currentWave + 1,
                                    messageToSend
                            })};

                            if (getSession()->getParam().getVerbose())
                                cout << "Broadcaster #" << getSession()->getRank() << " sends message of step number"
                                     << i << " from wave " << currentWave + 1 << " to broadcater "
                                     << /* //TODO Not right it is pow2... */ i << "(MESSAGE ONE WAVE IN ADVANCE)\n";;
                            peers[i + 1]->sendMsg(mes);
                            alreadySentNextWave[i] = true;
                        }
                    }
                }


            }

            break;
        }

        case DisconnectIntent : {
            auto bdi{deserializeStruct<BroadcasterDisconnectIntent>(msgString)};
            auto s{serializeStruct<StructAckDisconnectIntent>(StructAckDisconnectIntent{
                    MsgId::AckDisconnectIntent
            })};

            peer->sendMsg(s);
            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank() << " announces it will disconnect \n";;
            break;
        }

        default: {
            cerr << "ERROR\tBBOBBAlgoLayer: Unexpected msgId (" << static_cast<int>(msgId) << ")\n";
            exit(EXIT_FAILURE);
        }
    }

    return msgId == MsgId::AckDisconnectIntent;

}


bool BBOBBAlgoLayer::executeAndProducedStatistics() {

    bool verbose = getSession()->getParam().getVerbose();

    auto commLayer = getSession()->getCommLayer();
    auto sites = getSession()->getParam().getSites();
    int rank = getSession()->getRank();

    setBroadcasters(sites);

    int nbAwaitedConnections;
    int n = getSession()->getParam().getSites().size();
    if (n % 2 == 0) {
        nbAwaitedConnections = log2(n);
    } else {
        nbAwaitedConnections = floor(log2(n)) + 1;
    }
    cout << "Broadcaster#" << rank << " initHost\n";
    commLayer->initHost(get<PORT>(sites[rank]), nbAwaitedConnections, this);


    std::thread t([rank, verbose, sites, this]() {
        // Send RankInfo
        for (int power_of_2 = 1; power_of_2 < sites.size(); power_of_2 *= 2) {
            std::unique_ptr<CommPeer> cp = getSession()->getCommLayer()->connectToHost(
                    getSession()->getParam().getSites()[(power_of_2 + rank) %
                                                        getSession()->getParam().getSites().size()],
                    this);
            peers.push_back(std::move(cp));
        }

        for (int power_of_2 = 1, i = 0; power_of_2 < sites.size(); power_of_2 *= 2, i++) {
            if (verbose)
                cout << "Broadcaster#" << rank << " : Send RankInfo to broadcaster "
                     << (power_of_2 + rank) % getSession()->getParam().getSites().size() << "\n";
            auto s{serializeStruct<BroadcasterRankInfo>(BroadcasterRankInfo{
                    MsgId::RankInfo,
                    static_cast<unsigned char>(getSession()->getRank())
            })};
            peers[i]->sendMsg(s);
        }
        if (verbose)
            cout << "Broadcaster#" << rank << " : sent all messages\n";
    });


    if (verbose)
        cout << "Broadcaster#" << rank << " Wait for messages\n";
    commLayer->waitForMsg(/* //TODO Change with good value (floor (...)) */ 2);


    if (verbose)
        cout << "Broadcaster#" << rank
             << " Finished waiting for messages ==> Giving back control to SessionLayer\n";

    return false;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {

    auto bmtb{serializeStruct<Message>(Message{
            MsgId::MessageToReceive,
            static_cast<unsigned char>(getSession()->getRank()),
            ++seqNum,
            msg
    })};
    msgWaitingToBeBroadcasted.push_back(bmtb);
    if (messagesOfOneWave.empty()) {
        std::vector<std::string> myMessages;
        messagesOfOneWave[0] = myMessages;
    }
    messagesOfOneWave[0].push_back(bmtb);
    received[0] = true;

}

void BBOBBAlgoLayer::terminate() {
    auto s{serializeStruct<BroadcasterDisconnectIntent>(BroadcasterDisconnectIntent{MsgId::DisconnectIntent,
                                                                                    static_cast<unsigned char>(getSession()->getRank())})};
    for (int i = 0; i < (int) log2(getSession()->getParam().getSites().size()) + 1; i++) {
        peers[i]->sendMsg(s);
    }

}

std::string BBOBBAlgoLayer::toString() {
    return std::string();
}
