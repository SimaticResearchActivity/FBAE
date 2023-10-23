//
// Created by lardeur on 09/10/23.
//

#include "SessionLayer.h"
#include "BBOBBAlgoLayer.h"
#include "BBOBBAlgoLayerMsg.h"
#include "msgTemplates.h"

#include <tgmath.h>
#include <unistd.h>

using namespace std;
using namespace fbae_BBOBBAlgoLayer;


void BBOBBAlgoLayer::callbackHandleMessageAsHost(std::unique_ptr<CommPeer> peer, const std::string &msgString) {
    static atomic_int32_t seqNum{0};
    static atomic_int32_t nbConnectedBroadcasters{0};
    switch (BroadcasterMsgId broadcasterMsgTyp{ static_cast<BroadcasterMsgId>(msgString[0]) }; broadcasterMsgTyp) {

        case BroadcasterMsgId::RankInfo :
        {
            auto bri{deserializeStruct<BroadcasterRankInfo>(msgString)};
            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank()
                     << " : received RankInfo from broadcaster#" <<  static_cast<unsigned int>(bri.senderRank) << "\n";
            if (++nbConnectedBroadcasters == getBroadcasters().size()) {

                if (getSession()->getParam().getVerbose())
                    cout << "Broadcaster #" << getSession()->getRank() << " all broadcasters are connected \n";;
                auto s{serializeStruct<AllBroadcastersConnected >(AllBroadcastersConnected{BroadcasterMsgId::AllBroadcastersConnected})};
                getSession()->getCommLayer()->broadcastMsg(s);

            }
            break;
        }
        case BroadcasterMsgId::AckDisconnectIntent :
            peer->disconnect();
            break;
        case BroadcasterMsgId::MessageToReceive :
        {
            auto bmtb{deserializeStruct<BroadcasterMessageToSend>(msgString)};
            auto s {serializeStruct<BBOBBSendMessage>(BBOBBSendMessage{BroadcasterMsgId::ReceiveMessage,
                                                                                         bmtb.senderRank,
                                                                                         ++seqNum,
                                                                                         bmtb.sessionMsg})};
            peers[seqNum]->sendMsg(s);
            break;
        }
        default:
        {
            cerr << "ERROR\tBBOBBAlgoLayer / BBOBB : Unexpected broadcasterMsgTyp (" << static_cast<int>(broadcasterMsgTyp) << ")\n";
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

    //fork to handle connection while waiting for acknoledgment
    pid_t pid = fork();

    if (pid == -1) {
        if (verbose)
            cout << "Broadcaster" << rank << "didn't manage to fork";
        exit(EXIT_FAILURE);

    } else if (pid > 0) {
        if (verbose)
            cout << "Boradcaster#" << rank << " initHost\n";
        commLayer->initHost(get<PORT>(sites[rank]), floor(log2(getBroadcasters().size())) + 2, this);
        if (verbose)
            cout << "Broadcaster#" << rank << " Wait for messages\n";
        sleep(2);
        commLayer->waitForMsg(false, (int)log2(sites.size()) + 1);
        if (verbose)
            cout << "Broadcaster#" << rank
                 << " Finished waiting for messages ==> Giving back control to SessionLayer\n";

    } else {
        // Send RankInfo
        sleep(3);
        for (int i = 0; i < (int)log2(sites.size()) + 1; i++) {
            if (verbose)
                cout << "Broadcaster#" << rank << " : Send RankInfo to " << ((int)pow(2,i) + rank) % (int)getSession()->getParam().getSites().size()  << "\n";
            auto s{serializeStruct<BroadcasterRankInfo>(BroadcasterRankInfo{BroadcasterMsgId::RankInfo,
                                                                            static_cast<unsigned char>(getSession()->getRank())})};
            std::unique_ptr<CommPeer> cp =  getSession()->getCommLayer()->connectToHost(getSession()->getParam().getSites()[((int)pow(2,i) + rank) % (int)getSession()->getParam().getSites().size()], this);
            peers.push_back(std::move(cp));
            peers[i]->sendMsg(s);
        }
        if (verbose)
            cout << "Broadcaster#" << rank << " : sent all messages\n";


        //Temporary : disconnect just after connectiong
        sleep(3);
        for (int i = 0; i < (int)log2(sites.size()) + 1; i++) {
            //auto bdi{deserializeStruct<BroadcasterDisconnectIntent>(msgString)};
            //auto s{serializeStruct<AckDisconnectIntent>(SequencerAckDisconnectIntent{SequencerMsgId::AckDisconnectIntent})};
            //peer->sendMsg(s);
            peers[i]->disconnect();
            cout << "Broadcaster#" << rank
                 << " : disconnect from " << (int)(pow(2, i) + rank) %  (int)getSession()->getParam().getSites().size() << "\n";
        }

    }

    return true;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {
    auto bmtb {serializeStruct<BroadcasterMessageToSend>(BroadcasterMessageToSend{BroadcasterMsgId::MessageToReceive,
                                                                                            static_cast<unsigned char>(getSession()->getRank()),
                                                                                            msg})};
    peers[0]->sendMsg(bmtb);
}

void BBOBBAlgoLayer::terminate() {

}

std::string BBOBBAlgoLayer::toString() {
    return std::string();
}
