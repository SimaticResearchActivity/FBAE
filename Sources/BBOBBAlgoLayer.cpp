//
// Created by lardeur on 09/10/23.
//

#include "SessionLayer.h"
#include "BBOBBAlgoLayer.h"
#include "BBOBBAlgoLayerMsg.h"
#include "msgTemplates.h"

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

            cout << "Size of peers : " << peers.size() << " nb connected b : " << nbConnectedBroadcasters << "\n";
            nbConnectedBroadcasters++;

            if (nbConnectedBroadcasters == /* //TODO CHANGE THIS */ 2) {

                if (getSession()->getParam().getVerbose())
                    cout << "Broadcaster #" << getSession()->getRank() << " all broadcasters are connected \n";;

                getSession()->callbackInitDone();

                //Send Step 0 Message of wave 0
                if (getSession()->getParam().getVerbose())
                    cout << "Broadcaster #" << getSession()->getRank()
                         << " send step 0 of wave 0 to the first member of peers " << peers[0] <<"\n";;
                auto s{serializeStruct(StepMessage{
                        Step,
                        static_cast<unsigned char>(getSession()->getRank()),
                        0,
                        msgWaitingToBeBroadcasted
                })};
                peers[0]->sendMsg(s);
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
                     << " announces has received a step message of step number" << bmtb.stepNumber << " from broadcater"
                     << static_cast<unsigned int>(bmtb.senderRank) << "\n";;


            //Receive the right step
            if (bmtb.stepNumber == currentStep) {

                //C'est une step intermédiaire
                if (currentStep != peers.size()) {

                    currentStep++;

                    auto mes{serializeStruct(StepMessage{
                            Step,
                            static_cast<unsigned char>(getSession()->getRank()),
                            1,
                            msgWaitingToBeBroadcasted
                    })};

                    /*
                    if (getSession()->getParam().getVerbose())
                        cout << "Broadcaster #" << getSession()->getRank() << " sends message of step number"
                             << currentStep << " to broadcater" << currentStep << "\n";;
                    //peers[currentStep]->sendMsg(mes);
                    */

                } else {
                    //C'était le dernier message de la vague
                    for (const std::string &message: bmtb.msgBroadcasted) {
                        auto sbm{deserializeStruct<StructBroadcastMessage>(message)};
                        if (nextDeliver != sbm.seqNum) {
                            cerr << "ERROR\tSequencerAlgoLayer / Broadcaster #" << getSession()->getRank()
                                 << " : Received a totalOrderBroadcast message with seqNum=" << sbm.seqNum
                                 << " while nextDeliver=" << nextDeliver << "\n";
                            exit(EXIT_FAILURE);
                        }
                        getSession()->callbackDeliver(sbm.senderRank, sbm.seqNum, sbm.sessionMsg);
                        ++nextDeliver;
                        break;
                    }
                }

            } else if (bmtb.stepNumber == currentStep + 1) {
                // Receive one message ahead

                // Boucle while pour attendre que le message d'avant a bien a été reçu
            } else {

                printf("b : %d MY CURRENT STEP %d\n", getSession()->getRank(), currentStep);
                printf("b : %d MY MSGSTEP %d\n", getSession()->getRank(), bmtb.stepNumber);
                cout << "Message mauvais reçi de la part de " << static_cast<unsigned int>(bmtb.senderRank);

                cerr << "ERROR\tBBOBBAlgoLayer: Unexpected Step (" << bmtb.stepNumber << ")\n";
                exit(EXIT_FAILURE);
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
    auto bmtb{serializeStruct<StructMessageToBroadcast>(StructMessageToBroadcast{
            MsgId::MessageToBroadcast,
            static_cast<unsigned char>(getSession()->getRank()),
            msg
    })};
    msgWaitingToBeBroadcasted.push_back(bmtb);
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
