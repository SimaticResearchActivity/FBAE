#include <iostream>
#include <numeric>
#include <algorithm>

#include "../../SessionLayer/SessionLayer.h"
#include "Trains.h"
#include "../../msgTemplates.h"

using namespace std;

Trains::Trains(unique_ptr<CommLayer> commLayer): 
    AlgoLayer(move(commLayer))
{}

void Trains::callbackReceive(string&& serializedMessagePacket) {

    auto trainId{ static_cast<int>(serializedMessagePacket[0]) };
    auto trainClock{ static_cast<int>(serializedMessagePacket[1]) };

    if (trainClock > trainsClock[trainId]) {
        processTrain(move(serializedMessagePacket));
    }
    else {
        cerr << "ERROR\tLost train\n";
        // LOG4CXX_ERROR
    }
}

void Trains::processTrain(string&& serializedMessagePacket) {

    auto train = deserializeStruct<Train>(std::move(serializedMessagePacket));

    const rank_t rank = getSessionLayer()->getRank();
    const auto sitesCount = static_cast<uint32_t>(getSessionLayer()->getArguments().getSites().size());
    const rank_t nextRank = (rank + 1) % sitesCount;

    // Deliver messages that are not in the train anymore
    auto WagonIt = waitingWagons.begin();
    while (WagonIt != waitingWagons.end()) {
        // If it is the same train that brought the wagon, and the wagon is not in the train anymore, 
        // Then it means it has done a complete loop; so every machine should have received it
        if (train.id == WagonIt->trainId) {

            // Est-ce qu'il y a besoin de vérifier les messages du train ???

            for (auto const& message : WagonIt->batch.batchSessionMsg) {
                batchNoDeadlockCallbackDeliver(WagonIt->batch.senderPos, message);
            }
            // Remove the waiting wagon from the list
            WagonIt = waitingWagons.erase(WagonIt);
        }
        else { ++WagonIt; }
    }


    auto trainIt = train.wagons.begin();
    while (trainIt != train.wagons.end()) {
        // Add the train's wagons to waiting wagons
        waitingWagons.push_back(*trainIt);

        // Remove wagons which were sent by next
        if (trainIt->batch.senderPos == nextRank) {
            trainIt = train.wagons.erase(trainIt);
        }
        else { ++trainIt; }
    }

    // Add your own wagon to the train
    wagonToSend.trainId = train.id;
    for (auto const& message : batchGetBatchMsgs(rank)->batchSessionMsg) {
        wagonToSend.batch.batchSessionMsg.push_back(message);
    }
    train.wagons.push_back(wagonToSend);

    // Clear our wagon of messages (they have already been sent to the train)
    wagonToSend.batch.batchSessionMsg = {};

    trainsClock[train.id]++;
    train.clock++;

    const auto serialized = serializeStruct(train);
    getCommLayer()->multicastMsg(serialized);
}

void Trains::execute() {
    const rank_t rank = getSessionLayer()->getRank();
    const auto sitesCount = static_cast<uint32_t>(getSessionLayer()->getArguments().getSites().size());

    wagonToSend = { 0, { rank, {} } };

    std::vector<rank_t> broadcasters(sitesCount);
    std::iota(broadcasters.begin(), broadcasters.end(), 0);
    setBroadcastersGroup(std::move(broadcasters));

    getCommLayer()->openDestAndWaitIncomingMsg({ static_cast<rank_t>((rank + 1) % sitesCount)}, 1, this);
}

void Trains::totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) {
    wagonToSend.batch.batchSessionMsg.push_back(sessionMsg);

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

std::string Trains::toString() {
    return "Trains";
}

void Trains::terminate() {
    getCommLayer()->terminate();
}
