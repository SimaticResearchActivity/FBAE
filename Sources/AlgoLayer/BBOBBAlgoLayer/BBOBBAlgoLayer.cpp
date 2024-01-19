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

                //Necessary so that every participant received all of their rank info before the others messages
                sleep(2);

                //Initialize all vectors that will be used
                messagesOfOneWave.resize(peers.size() + 1);
                messagesOfNextWave.resize(peers.size() + 1);
                for (int i = 0; i < peers.size() + 1; i++) {
                    std::vector<std::string> vectorToAcceptMessagesOneWave;
                    std::vector<std::string> vectorToAcceptMessagesNextWave;
                    messagesOfOneWave[i] = vectorToAcceptMessagesOneWave;
                    messagesOfNextWave[i] = vectorToAcceptMessagesNextWave;
                }
                alreadySent.resize(peers.size());
                for (int i = 0; i < peers.size(); i++) {
                    alreadySent[i] = false;
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
                messagesOfOneWave[0] = msgWaitingToBeBroadcasted;
                msgWaitingToBeBroadcasted.clear();
                received[0] = true;
                peers[0]->sendMsg(s);
            }
            break;
        }

        case AckDisconnectIntent : {
            auto mes{deserializeStruct<StructAckDisconnectIntent>(msgString)};
            cout << "RECEIVED ACK NTENT with rankInPeersVector " << mes.rankInPeersVector << "from "
                 << static_cast<unsigned int>(mes.senderRank) << "\n";
            peers[mes.rankInPeersVector]->disconnect();
            break;
        }


        case Step : {

            auto bmtb{deserializeStruct<StepMessage>(msgString)};

            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank() << " received a step message from broadcater "
                     << static_cast<unsigned int>(bmtb.senderRank) << " of step " << bmtb.stepNumber << " and wave "
                     << bmtb.wave << "\n";

            if (currentWave == bmtb.wave) {

                messagesOfOneWave[bmtb.stepNumber + 1] = bmtb.msgBroadcasted;
                received[bmtb.stepNumber + 1] = true;

                bool empty = false;

                for (int i = 1; i < messagesOfOneWave.size() - 1; i++) {
                    if (received[i] && received[0]) {
                        if (!alreadySent[i]) {
                            std::vector<std::string> messageToSend;
                            for (int j = 0; j < i + 1; j++) {
                                messageToSend.insert(messageToSend.end(), messagesOfOneWave[j].begin(),
                                                     messagesOfOneWave[j].end());
                            }
                            if (getSession()->getParam().getVerbose())
                                cout << "Broadcaster #" << getSession()->getRank()
                                     << " sends message of step number"
                                     << i << " from wave " << currentWave << " to broadcater"
                                     << /* //TODO Not right it is pow2... */ i << "\n";
                            auto mes{serializeStruct(StepMessage{
                                    Step,
                                    static_cast<unsigned char>(getSession()->getRank()),
                                    i,
                                    currentWave,
                                    messageToSend
                            })};
                            peers[i + 1]->sendMsg(mes);
                            alreadySent[i] = true;
                        }
                    } else {
                        //We received a step in advance so we quit the loop
                        empty = true;
                        break;
                    }
                }

                //After all the messages of one wave have been sent we deliver the messages
                if (!empty && received.back()) {

                    std::vector<std::vector<std::string>> messagesToDeliver;
                    messagesToDeliver.resize(getSession()->getParam().getSites().size());
                    for (int i = 0; i < peers.size() + 1; i++) {
                        std::vector<std::string> vector;
                        messagesToDeliver[i] = vector;
                    }

                    //The messageOfOneWave.size() -1 messages can be sorted by broadcasters
                    for (int i = 0; i < messagesOfOneWave.size() - 1; i++) {
                        for (std::string &message: messagesOfOneWave[i]) {
                            auto s{deserializeStruct<Message>(message)};
                            messagesToDeliver[s.senderRank].push_back(s.sessionMsg);
                        }
                    }

                    //The messageOfOneWave.size() message might have duplicates
                    if (!messagesOfOneWave.back().empty()) {
                        int sender = -1;
                        for (int i = 0; i < messagesOfOneWave.back().size(); i++) {
                            auto s{deserializeStruct<Message>(messagesOfOneWave.back()[i])};
                            if (messagesToDeliver[s.senderRank].empty() || sender == s.senderRank) {
                                messagesToDeliver[s.senderRank].push_back(s.sessionMsg);
                                sender = s.senderRank;
                            }
                        }
                    }

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
                    }
                    for (int i = 0; i < peers.size() + 1; i++) {
                        received[i] = false;
                        received[i] = receivedNextWave[i];
                        receivedNextWave[i] = false;
                    }

                    //Delivering the message in the order
                    for (int i = 0; i < messagesToDeliver.size(); i++) {
                        for (std::string message: messagesToDeliver[i]) {
                            getSession()->callbackDeliver(i, seqNum++, message);
                        }
                    }

                    currentWave++;
                    //All messages were delivered so we can launch another wave
                    cout << "Broadcaster #" << getSession()->getRank()
                         << " send step 0 of wave " << currentWave << " to the first member of peers "
                         << peers[0]
                         << "\n";;
                    auto s{serializeStruct(StepMessage{
                            Step,
                            static_cast<unsigned char>(getSession()->getRank()),
                            0,
                            currentWave,
                            msgWaitingToBeBroadcasted
                    })};
                    messagesOfOneWave[0] = msgWaitingToBeBroadcasted;
                    msgWaitingToBeBroadcasted.clear();
                    received[0] = true;
                    if (sendWave)
                        peers[0]->sendMsg(s);

                    //We send the messages from the wave we received in advance
                    for (int i = 1; i < messagesOfOneWave.size() - 1; i++) {
                        if (received[i] && received[0]) {
                            if (!alreadySent[i]) {
                                std::vector<std::string> messageToSend;
                                for (int j = 0; j < i + 1; j++) {
                                    messageToSend.insert(messageToSend.end(), messagesOfOneWave[j].begin(),
                                                         messagesOfOneWave[j].end());
                                }
                                if (getSession()->getParam().getVerbose())
                                    cout << "Broadcaster #" << getSession()->getRank()
                                         << " sends message of step number"
                                         << i << " from wave " << currentWave << " to broadcater"
                                         << /* //TODO Not right it is pow2... */ i << "\n";
                                auto mes{serializeStruct(StepMessage{
                                        Step,
                                        static_cast<unsigned char>(getSession()->getRank()),
                                        i,
                                        currentWave,
                                        messageToSend
                                })};
                                peers[i + 1]->sendMsg(mes);
                                alreadySent[i] = true;
                            }
                        }
                    }
                }
            } else {
                cout << "wave in advance \n";
                //Message is early and needs to be treated in the next wave
                messagesOfNextWave[bmtb.stepNumber + 1] = bmtb.msgBroadcasted;
                receivedNextWave[bmtb.stepNumber + 1] = true;
            }

            break;
        }

        case DisconnectIntent : {
            auto bdi{deserializeStruct<BroadcasterDisconnectIntent>(msgString)};
            auto s{serializeStruct<StructAckDisconnectIntent>(StructAckDisconnectIntent{
                    MsgId::AckDisconnectIntent,
                    static_cast<unsigned char>(getSession()->getRank()),
                    bdi.rankInPeersVector
            })};
            cout << "RECEIVED DICONNECTINTENT FROM " << static_cast<int>(bdi.senderRank)
                 << " and send ACKDISONNECTINTENT TO (i) " << bdi.rankInPeersVector << "\n";
            //TODO ERROR
            peers[bdi.senderRank]->sendMsg(s);
            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << static_cast<unsigned int>(bdi.senderRank)
                     << " announces it will disconnect \n";
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

    int numberOfPeers;
    int n = getSession()->getParam().getSites().size();
    if (n % 2 == 0) {
        numberOfPeers = log2(n);
    } else {
        numberOfPeers = floor(log2(n)) + 1;
    }
    cout << "Broadcaster#" << rank << " initHost\n";
    commLayer->initHost(get<PORT>(sites[rank]), numberOfPeers, this);


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
    commLayer->waitForMsg(numberOfPeers);

    if (verbose)
        cout << "Broadcaster#" << rank
             << " Finished waiting for messages ==> Giving back control to SessionLayer\n";

    t.join();

    return true;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {
    //Message is received by the broadcaster and will be sent in the next wave at step 0
    auto bmtb{serializeStruct<Message>(Message{
            MsgId::MessageToReceive,
            static_cast<unsigned char>(getSession()->getRank()),
            msg
    })};
    msgWaitingToBeBroadcasted.push_back(bmtb);
}

void BBOBBAlgoLayer::terminate() {

    //Empecher un broadcaster de se deconnecter pour avoir les infos
    if (getSession()->getRank() != 0) {
        sendWave = 1;
        for (int i = 0; i < peers.size(); i++) {
            cout << "peer number " << i << " disconnect\n";
            peers[i]->disconnect();
        }
    }


    /*
    for (int i = 0; i < peers.size(); i++) {
        cout << "SEND DISCONNECTINTENT" << i << "\n";
        auto s{serializeStruct<BroadcasterDisconnectIntent>(BroadcasterDisconnectIntent{
            MsgId::DisconnectIntent,
            static_cast<unsigned char>(getSession()->getRank()),
            i})};
        peers[i]->sendMsg(s);
    }
     */

}

std::string BBOBBAlgoLayer::toString() {
    return std::string();
}
