#include <iostream>
#include <numeric>
#include <algorithm>

#include "../../SessionLayer/SessionLayer.h"
#include "Trains.h"
#include "../../msgTemplates.h"

#include "Logger/Logger.h"

Trains::Trains(std::unique_ptr<CommLayer> commLayer) noexcept: pendingWagons({}), AlgoLayer(std::move(commLayer)) { }

void Trains::execute() noexcept {
    const rank_t rank = getSessionLayer()->getRank();
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();

    wagonToSend = {
            .sender = rank,
            .msgs = {}
    };

    std::vector<rank_t> broadcasters(sitesCount);
    std::iota(broadcasters.begin(), broadcasters.end(), 0);
    setBroadcastersGroup(std::move(broadcasters));

    getCommLayer()->openDestAndWaitIncomingMsg({ static_cast<rank_t>((rank + 1) % sitesCount)}, 1, this);
}

void Trains::totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) noexcept {
    wagonToSend.msgs.push_back({ sessionMsg, clock });

    rank_t rank = getSessionLayer()->getRank();

    if (rank == 0 && sessionMsg->msgId == fbae_SessionLayer::SessionMsgId::FirstBroadcast) {
        const Train train = {
                .id = 0,
                .clock = 0,
                .wagons = { }
        };

        callbackReceive(serializeStruct(train));
    }
}

std::string Trains::toString() noexcept {
    return "Trains";
}

void Trains::terminate() noexcept {
    getCommLayer()->terminate();
}

void Trains::callbackReceive(std::string && serializedMessagePacket) noexcept {
    const rank_t rank = getSessionLayer()->getRank();
    const uint32_t sitesCount = getSessionLayer()->getArguments().getSites().size();
    const rank_t nextRank = (rank + 1) % sitesCount;

    auto logger = Logger::instanceOnSite("Trains::callbackReceive", rank);

    auto train = deserializeStruct<Train>(std::move(serializedMessagePacket));

    // // Add all the train's wagons to our list of pending wagons in clock order
    // for (const auto &wagon : train.wagons) {
    //     pendingWagons.insert(std::upper_bound(pendingWagons.begin(), pendingWagons.end(), wagon, [](const Wagon &a, const Wagon &b) {
    //         return b.clock > a.clock;
    //     }), wagon);
    // }

    for (const auto &wagon : train.wagons) {
        pendingWagons.push_back(wagon);
    }

    // remove messages we sent
    train.wagons.erase(std::remove_if(train.wagons.begin(), train.wagons.end(), [rank](const Wagon &w) { return w.sender == rank; }), train.wagons.end());

    // Add our wagon of messages to the train
    wagonToSend.clock = train.clock;
    train.wagons.push_back(wagonToSend);

    // clear our wagon of messages (they have already been sent to the train)
    wagonToSend.msgs = {};

    for (const auto &wagon: pendingWagons) {
        for (const auto &message: wagon.msgs) {
            getSessionLayer()->callbackDeliver(wagon.sender, message.message);
        }
    }

    pendingWagons = {};

    clock++;
    train.clock++;
    const auto serialized = serializeStruct(train);
    getCommLayer()->multicastMsg(serialized);
}