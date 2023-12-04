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
    thread_local atomic_int32_t seqNum{0};
    thread_local atomic_int32_t nbConnectedBroadcasters{0};
    MsgId msgId{static_cast<MsgId>(msgString[0])};
    switch (msgId) {
        using
        enum MsgId;

        case RankInfo : {
            auto bri{deserializeStruct<BroadcasterRankInfo>(msgString)};
            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank()
                     << " : received RankInfo from broadcaster#" << static_cast<unsigned int>(bri.senderRank) << "\n";
            if (++nbConnectedBroadcasters == 2) {

                //if (getSession()->getParam().getVerbose())
                //    cout << "Broadcaster #" << getSession()->getRank() << " all broadcasters are connected \n";;
                //auto s{serializeStruct<AllBroadcastersConnected >(AllBroadcastersConnected{BroadcasterMsgId::AllBroadcastersConnected})};
                //getSession()->getCommLayer()->broadcastMsg(s);
                getSession()->callbackInitDone();
            }
            break;
        }
        case AckDisconnectIntent :
            peer->disconnect();
            break;
        case Step : {
            auto bmtb{deserializeStruct<BroadcasterMessageToSend>(msgString)};
            auto s{serializeStruct<BBOBBSendMessage>(BBOBBSendMessage{ReceiveMessage,
                                                                      bmtb.senderRank,
                                                                      ++seqNum,
                                                                      bmtb.sessionMsg})};
            peers[seqNum]->sendMsg(s);
            break;
        }
        case DisconnectIntent : {
            auto bdi{deserializeStruct<BroadcasterDisconnectIntent>(msgString)};
            auto s{serializeStruct<StructAckDisconnectIntent>(StructAckDisconnectIntent{MsgId::AckDisconnectIntent})};

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
    vector<tuple<basic_string<char>, int>> broadcaster = getBroadcasters();

    cout << "Broadcaster#" << rank << " initHost\n";
    commLayer->initHost(get<PORT>(sites[rank]), floor(log2(broadcaster.size())) + 1, this);

    std::thread t([rank, commLayer, verbose, sites, broadcaster, this]() {
        // Send RankInfo
        for (int power_of_2 = 1; power_of_2 < sites.size(); power_of_2 *= 2) {
            std::unique_ptr<CommPeer> cp = getSession()->getCommLayer()->connectToHost(
                    getSession()->getParam().getSites()[(int) (power_of_2 + rank) %
                                                        (int) getSession()->getParam().getSites().size()], this);
            peers.push_back(std::move(cp));
        }
        this_thread::sleep_for(5000ms);
        for (int i = 0; i < peers.size(); i++) {
            if (verbose)
                cout << "Broadcaster#" << rank << " : Send RankInfo to " << i << "\n";
            auto s{serializeStruct<BroadcasterRankInfo>(BroadcasterRankInfo{MsgId::RankInfo,
                                                                            static_cast<unsigned char>(getSession()->getRank())})};
            peers[i]->sendMsg(s);
        }
        if (verbose)
            cout << "Broadcaster#" << rank << " : sent all messages\n";
    });


    if (verbose)
        cout << "Broadcaster#" << rank << " Wait for messages\n";
    commLayer->waitForMsg((int) log2(sites.size()) + 1);
    if (verbose)
        cout << "Broadcaster#" << rank << " Finished waiting for messages ==> Giving back control to SessionLayer\n";

    t.join();
    cout << rank << " est arrivé là\n";

    sleep(3);
    for (int power_of_2 = 1; power_of_2 < sites.size(); power_of_2 *= 2) {
        peers.back()->disconnect();
        cout << "Broadcaster#" << rank << " : disconnect from"
             << (int) power_of_2 % (int) getSession()->getParam().getSites().size() << "\n";

    }


    return false;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {
    auto bmtb{serializeStruct<BroadcasterMessageToSend>(BroadcasterMessageToSend{MsgId::Step,
                                                                                 static_cast<unsigned char>(getSession()->getRank()),
                                                                                 msg})};
    peers[0]->sendMsg(bmtb);
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
