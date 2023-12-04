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


void BBOBBAlgoLayer::callbackHandleMessageAsHost(std::unique_ptr<CommPeer> peer, const std::string &msgString) {
    thread_local atomic_int32_t seqNum{0};
    thread_local atomic_int32_t nbConnectedBroadcasters{0};
    switch (BroadcasterMsgId broadcasterMsgTyp{static_cast<BroadcasterMsgId>(msgString[0])}; broadcasterMsgTyp) {

        case BroadcasterMsgId::RankInfo : {
            auto bri{deserializeStruct<BroadcasterRankInfo>(msgString)};
            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank()
                     << " : received RankInfo from broadcaster#" << static_cast<unsigned int>(bri.senderRank) << "\n";
            if (++nbConnectedBroadcasters == peers.size()) {

                if (getSession()->getParam().getVerbose())
                    cout << "Broadcaster #" << getSession()->getRank() << " all broadcasters are connected \n";;
                auto s{serializeStruct<AllBroadcastersConnected >(AllBroadcastersConnected{BroadcasterMsgId::AllBroadcastersConnected})};
                getSession()->getCommLayer()->broadcastMsg(s);
                getSession()->callbackInitDone();
                //auto step0 {};
            }
            break;
        }
        case BroadcasterMsgId::AckDisconnectIntent :
            peer->disconnect();
            break;
        case BroadcasterMsgId::Step : {
            auto bmtb{deserializeStruct<BroadcasterMessageToSend>(msgString)};
            auto s{serializeStruct<BBOBBSendMessage>(BBOBBSendMessage{BroadcasterMsgId::ReceiveMessage,
                                                                      bmtb.senderRank,
                                                                      ++seqNum,
                                                                      bmtb.sessionMsg})};
            peers[seqNum]->sendMsg(s);
            break;
        }

        case BroadcasterMsgId::AllBroadcastersConnected :
            getSession()->callbackInitDone();
            break;

        case BroadcasterMsgId::DisconnectIntent : {
            auto bdi{deserializeStruct<BroadcasterDisconnectIntent>(msgString)};
            auto s{serializeStruct<AckDisconnectIntent>(AckDisconnectIntent{BroadcasterMsgId::AckDisconnectIntent})};
            peer->sendMsg(s);
            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank() << " announces it will disconnect \n";;
            break;
        }
        default: {
            cerr << "ERROR\tBBOBBAlgoLayer / BBOBB : Unexpected broadcasterMsgTyp ("
                 << static_cast<int>(broadcasterMsgTyp) << ")\n";
            exit(EXIT_FAILURE);
        }
    }

}

void BBOBBAlgoLayer::callbackHandleMessageAsNonHostPeer(std::unique_ptr<CommPeer> peer, const std::string &msgString) {

}


bool BBOBBAlgoLayer::executeAndProducedStatistics() {

    bool verbose = getSession()->getParam().getVerbose();

    auto commLayer = getSession()->getCommLayer();
    auto sites = getSession()->getParam().getSites();
    int rank = getSession()->getRank();
    vector<tuple<basic_string<char>, int>> broadcaster = getBroadcasters();

    cout << "Broadcaster#" << rank << " initHost\n";
    commLayer->initHost(get<PORT>(sites[rank]), floor(log2(broadcaster.size())) + 1, this);

    std::thread t([rank, commLayer, verbose, sites, broadcaster, this]() {
        // Send RankInfo
        for (int power_of_2 = 1; power_of_2 < sites.size(); power_of_2 *= 2) {
            if (verbose)
                cout << "Broadcaster #" << rank << " : Connect to  \n"
                     << (int) power_of_2 % (int) getSession()->getParam().getSites().size() << "\n";;
            std::unique_ptr<CommPeer> cp = getSession()->getCommLayer()->connectToHost(
                    getSession()->getParam().getSites()[(int) (power_of_2 + rank) %
                                                        (int) getSession()->getParam().getSites().size()], this);
            peers.push_back(std::move(cp));
        }

        this_thread::sleep_for(5000ms);

        for (int power_of_2 = 1, i = 0; power_of_2 < sites.size(); power_of_2 *= 2, i++) {
            if (verbose)
                cout << "Broadcaster#" << rank << " : Send RankInfo to "
                     << (int) power_of_2 % (int) getSession()->getParam().getSites().size() << "\n";;
            auto s{serializeStruct<BroadcasterRankInfo>(BroadcasterRankInfo{BroadcasterMsgId::RankInfo,
                                                                            static_cast<unsigned char>(getSession()->getRank())})};
            peers[i]->sendMsg(s);
        }
        if (verbose)
            cout << "Broadcaster#" << rank << " : sent all messages\n";



    });


    if (verbose)
        cout << "Broadcaster#" << rank << " Wait for messages\n";
    commLayer->waitForMsg(false, (int) log2(sites.size()) + 1);
    if (verbose)
        cout << "Broadcaster#" << rank << " Finished waiting for messages ==> Giving back control to SessionLayer\n";


    sleep(3);
    for (int power_of_2 = 1, i = 0; power_of_2 < sites.size(); power_of_2 *= 2, i++) {
        peers[i]->disconnect();
        cout << "Broadcaster#" << rank << " : disconnect from"
             << (int) power_of_2 % (int) getSession()->getParam().getSites().size() << "\n";
    }


    return false;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {
    auto bmtb{serializeStruct<BroadcasterMessageToSend>(BroadcasterMessageToSend{BroadcasterMsgId::Step,
                                                                                 static_cast<unsigned char>(getSession()->getRank()),
                                                                                 msg})};
    peers[0]->sendMsg(bmtb);
}

void BBOBBAlgoLayer::terminate() {
    auto s{serializeStruct<BroadcasterDisconnectIntent>(BroadcasterDisconnectIntent{BroadcasterMsgId::DisconnectIntent,
                                                                                    static_cast<unsigned char>(getSession()->getRank())})};
    for (int i = 0; i < (int) log2(getSession()->getParam().getSites().size()) + 1; i++) {
        peers[i]->sendMsg(s);
    }

}

std::string BBOBBAlgoLayer::toString() {
    return std::string();
}
