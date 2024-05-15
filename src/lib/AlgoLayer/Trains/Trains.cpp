#include <iostream>
#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "Trains.h"
#include "../../msgTemplates.h"

#include "Logger/Logger.h"
#include <assert.h>

Trains::Trains(std::unique_ptr<CommLayer> commLayer) noexcept:AlgoLayer(std::move(commLayer)) {
    nbJoin = 1;
    idLast = NB_TR - 1;
    for (int i = 0; i < NB_TR; ++i) {
        lastTrains[i] = {.initialize = false};
        for (int j = 0; j < NB_ROT; ++j) {
            receivedWagons[i][j] = {};
        }
    }
    initDone = false;
}

void Trains::execute() noexcept {
    wagonToSend = {
            .sender = getSessionLayer()->getRank(), .rotation = 0, .msgs = {}
    };
    const rank_t rank = getSessionLayer()->getRank();
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    std::vector<rank_t> broadcasters(sitesCount);
    std::iota(broadcasters.begin(), broadcasters.end(), 0);
    setBroadcastersGroup(std::move(broadcasters));

    getCommLayer()->openDestAndWaitIncomingMsg({ static_cast<rank_t>((rank + 1) % sitesCount)}, 1, this);
}

void Trains::totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) noexcept {
    wagonToSend.msgs.push_back(sessionMsg);
}

std::string Trains::toString() noexcept {
    return "Trains";
}

void Trains::terminate() noexcept {
    getCommLayer()->terminate();
}

void Trains::callbackReceive(std::string && serializedMessagePacket) noexcept {
    auto train = deserializeStruct<Train>(std::move(serializedMessagePacket));
    int id = train.id;
    rank_t rank = getSessionLayer()->getRank();
    int siteCount = getSessionLayer()->getArguments().getSites().size();
    if (initDone) {
        if (id == (idLast + 1) % NB_ROT && train.clock > lastTrains[id].clock) {
            int rotation = train.rotation;
            if (rotation == lastTrains[id].rotation) {
                rotation = (rotation + 1) % NB_ROT;
            }
            int r = (rotation + 1) % NB_ROT;
            for (int i = 0; i < receivedWagons[id][r].size(); ++i) {
                std::vector<rank_t> ranks = {};
                for (int j = 0; j < receivedWagons[id][r][i].msgs.size(); ++j) {
                    ranks.push_back(receivedWagons[id][r][i].sender);
                }
                UTODeliver(receivedWagons[id][r][i].msgs, std::move(ranks));
            }
            receivedWagons[id][r] = {};
            lastTrains[id].clock = train.clock + 1;
            lastTrains[id].rotation = rotation;
            lastTrains[id].wagons = {};
            for (const auto &w : train.wagons) {
                receivedWagons[id][w.rotation].push_back(w);
                if (w.sender != (rank + 1) % siteCount) {
                    lastTrains[id].wagons.push_back(w);
                }
            }
            wagonToSend.rotation = rotation;
            lastTrains[id].wagons.push_back(wagonToSend);
            receivedWagons[id][rotation].push_back(wagonToSend);
            wagonToSend.msgs = {};
        }
    } else {
        lastTrains[id] = train;
        bool done = true;
        for (int i = 0; i < NB_TR; ++i) {
            if (!lastTrains[i].initialize) {
                done = false;
                break;
            }
        }
        if (done) {
            initDone = true;
        }
    }
    const auto serialized = serializeStruct<Train>(train);
    getCommLayer()->multicastMsg(serialized);
    idLast = id;
}



void Trains::UTODeliver(std::vector<fbae_SessionLayer::SessionMsg> messages, std::vector<rank_t> ranks) {
    assert(messages.size() == ranks.size() && "messages and ranks must be of the same size.");
    for (int i = 0; i < messages.size(); ++i) {
        getSessionLayer()->callbackDeliver(ranks[i], messages[i]);
    }
}