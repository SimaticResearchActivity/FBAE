//
// Created by lardeur on 09/10/23.
//

#include "SessionLayer.h"
#include "BBOBBAlgoLayer.h"
#include "BBOBBAlgoLayerMsg.h"
#include "msgTemplates.h"

#include <tgmath.h>


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
                cout << "\tSequencerAlgoLayer / Sequencer : Broadcaster #" << static_cast<unsigned int>(bri.senderRank) << " is connected to sequencer.\n";
            if (++nbConnectedBroadcasters == getBroadcasters().size()) {
                /*
                if (getSession()->getParam().getVerbose())
                    cout << "\tSequencerAlgoLayer / Sequencer : All broadcasters are connected: Broadcast AllBroadcastersConnected\n";
                auto s{serializeStruct<SequencerAllBroadcastersConnected >(SequencerAllBroadcastersConnected{SequencerMsgId::AllBroadcastersConnected})};
                getSession()->getCommLayer()->broadcastMsg(s);
            */
            }
            break;
        }
    }

}

void BBOBBAlgoLayer::callbackHandleMessageAsNonHostPeer(std::unique_ptr<CommPeer> peer, const std::string &msgString) {

}

bool BBOBBAlgoLayer::executeAndProducedStatistics() {

    bool verbose = getSession()->getParam().getVerbose();

    auto commLayer = getSession()->getCommLayer();
    auto sites = getSession()->getParam().getSites();

    //Alll broadcasters establish connection with each other
    if (verbose)
        cout << "Boradcatser" << getSession()->getRank() << " establish connexion with other broadcasters\n";
    commLayer->initHost(get<PORT>(sites[getSession()->getRank()]), floor(log2(getBroadcasters().size())) + 1, this);

    //fork to handle connection while waiting for acknoledgment
    pid_t pid = fork();

    if (pid == -1) {
        if (verbose)
            cout << "Broadcaster" << getSession()->getRank() << "didn't manage to fork";
        exit(EXIT_FAILURE);

    } else if (pid > 0) {
        // Send RankInfo to sequencer
        for (int i = 0; i < (int)log2(sites.size()) + 1; i++) {
            if (getSession()->getParam().getVerbose())
                cout << "Broadcaster #" << getSession()->getRank()
                     << " : Send RankInfo to " << i << "\n";
            auto s{serializeStruct<BroadcasterRankInfo>(BroadcasterRankInfo{BroadcasterMsgId::RankInfo,
                                                                            static_cast<unsigned char>(getSession()->getRank())})};
            std::unique_ptr<CommPeer> cp =  getSession()->getCommLayer()->connectToHost(getSession()->getParam().getSites()[pow(2,i)], this);
            cp->disconnect();
            peers.push_back(std::move(cp));
            peers[i]->sendMsg(s);
        }
        if (verbose)
            cout << "Broadcaster #" << getSession()->getRank()
                 << " : sent all messages\n";
        for (int i = 0; i < (int)log2(sites.size()) + 1; i++) {
            peers[i]->disconnect();
            cout << "Broadcaster #" << getSession()->getRank()
                 << " : disconnect from" << i << "\n";
        }

    } else {
        if (verbose)
            cout << "Broadcaster" << getSession()->getRank() << "Wait for messages\n";
        commLayer->waitForMsg(false, (int)log2(sites.size()) + 1);
        if (verbose)
            cout << "Broadcaster " << getSession()->getRank()
                 << " Finished waiting for messages ==> Giving back control to SessionLayer\n";
    }

    return false;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {

}

void BBOBBAlgoLayer::terminate() {

}

std::string BBOBBAlgoLayer::toString() {
    return std::string();
}
