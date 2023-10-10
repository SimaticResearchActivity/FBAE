//
// Created by lardeur on 09/10/23.
//

#include "SessionLayer.h"
#include "BBOBBAlgoLayer.h"
#include "BBOBBAlgoLayerMsg.h"
#include "msgTemplates.h"


using namespace std;
using namespace fbae_BBOBBAlgoLayer;


void BBOBBAlgoLayer::callbackHandleMessageAsHost(std::unique_ptr<CommPeer> peer, const std::string &msgString) {

}

void BBOBBAlgoLayer::callbackHandleMessageAsNonHostPeer(std::unique_ptr<CommPeer> peer, const std::string &msgString) {

}

bool BBOBBAlgoLayer::executeAndProducedStatistics() {

    bool verbose = getSession()->getParam().getVerbose();

    broadcasters = getSession()->getParam().getSites();
    setBroadcasters(broadcasters);

    auto commLayer = getSession()->getCommLayer();
    auto sites = getSession()->getParam().getSites();

    //Alll broadcasters establish connection with each other
    if (verbose)
        cout << "Boradcatser" << getSession()->getRank() << " establish connexion with other broadcasters\n";
    commLayer->initHost(get<PORT>(sites.back()), getBroadcasters().size(), this);
    if (verbose)
        cout << "Broadcaster " << getSession()->getRank() <<" Finished waiting for messages ==> Giving back control to SessionLayer\n";

    return false;
}

void BBOBBAlgoLayer::totalOrderBroadcast(const std::string &msg) {

}

void BBOBBAlgoLayer::terminate() {

}

std::string BBOBBAlgoLayer::toString() {
    return std::string();
}
