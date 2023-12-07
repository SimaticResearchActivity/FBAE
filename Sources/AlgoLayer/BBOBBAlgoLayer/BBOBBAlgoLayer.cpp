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


            if (++nbConnectedBroadcasters == /* //TODO CHANGE THIS */ 2) {

                if (getSession()->getParam().getVerbose())
                    cout << "Broadcaster #" << getSession()->getRank() << " all broadcasters are connected \n";;

                sleep(2);
                messagesOfOneWave.resize(peers.size() + 1);
                for (int i = 0; i < peers.size(); i++) {
                    alreadySent.push_back(false);
                }
                getSession()->callbackInitDone();



                //Send Step 0 Message of wave 0
                if (getSession()->getParam().getVerbose())
                    cout << "Broadcaster #" << getSession()->getRank()
                         << " send step 0 of wave 0 to the first member of peers " << peers[0] << "\n";;
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
                     << " of step " << bmtb.stepNumber << " and wave " << currentWave << "\n";

            messagesOfOneWave[bmtb.stepNumber + 1] = bmtb.msgBroadcasted;

            bool empty = false;

            for (int i = 1; i < messagesOfOneWave.size() - 1; i++) {
                if (!messagesOfOneWave[i].empty()) {
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
                                 << i << " from wave " << currentWave << " to broadcater" << /* //TODO Not right it is pow2... */ i << "\n";;
                        peers[i + 1]->sendMsg(mes);
                        alreadySent[i] = true;
                    }
                } else {
                    empty = true;
                    break;
                }
            }

            //After all the messages of one wave have been sent we deliver the messages
            if (!empty && !messagesOfOneWave.back().empty()) {
                //The n-1 messages have to be delivered
                for (int i = 0; i < messagesOfOneWave.size() - 1; i++) {
                    for (std::string &message: messagesOfOneWave[i]) {
                        auto s{deserializeStruct<Message>(message)};
                        getSession()->callbackDeliver(s.senderRank, s.seqNum, s.sessionMsg);
                    }
                }

                //The nth message might have duplicates
                //Function to get the number of messages we have to deliver
                int numberOfDelivers;
                int n = getSession()->getParam().getSites().size();
                if (n % 2 == 0) {
                    numberOfDelivers = floor(log2(n));
                } else {
                    numberOfDelivers = n - pow(2,floor(log2(n)));
                }
                for (int i = 0; i < numberOfDelivers; i++) {
                    auto s{deserializeStruct<Message>(messagesOfOneWave.back()[i])};
                    getSession()->callbackDeliver(s.senderRank, s.seqNum, s.sessionMsg);
                }

                //Clear the vectirs we used and
                messagesOfOneWave.clear();
                alreadySent.clear();
                messagesOfOneWave.resize(peers.size() + 1);
                for (int i = 0; i < peers.size(); i++) {
                    alreadySent.push_back(false);
                }

                //All messages were delivered so we can launch another wave
                auto s{serializeStruct(StepMessage{
                        Step,
                        static_cast<unsigned char>(getSession()->getRank()),
                        0,
                        currentWave++,
                        msgWaitingToBeBroadcasted
                })};
                cout << "Broadcaster #" << getSession()->getRank()
                     << " send step 0 of wave " << currentWave <<" to the first member of peers " << peers[0] << "\n";;
                peers[0]->sendMsg(s);
                msgWaitingToBeBroadcasted.clear();
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

    cout << "Broadcaster#" << rank << " initHost\n";
    commLayer->initHost(get<PORT>(sites[rank]), /* //TODO Change to floor etc... */2, this);


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

    cout << rank << " est arrivé là\n";

    sleep(2);

    for (int power_of_2 = 1, i = 0; power_of_2 < sites.size(); power_of_2 *= 2, i++) {
        peers[i]->disconnect();
        cout << "Broadcaster#" << rank << " : disconnect from"
             << (int) power_of_2 % (int) getSession()->getParam().getSites().size() << "\n";

    }


    return false;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {
    auto bmtb{serializeStruct<Message>(Message{
            MsgId::MessageToReceive,
            static_cast<unsigned char>(getSession()->getRank()),
            seqNum++,
            msg
    })};
    msgWaitingToBeBroadcasted.push_back(bmtb);
    if (messagesOfOneWave[0].empty()) {
        std::vector<std::string> myMessages;
        messagesOfOneWave[0] = myMessages;
    }
    messagesOfOneWave[0].push_back(bmtb);
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
