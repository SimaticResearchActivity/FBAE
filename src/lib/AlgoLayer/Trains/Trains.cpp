#include <iostream>
#include <numeric>
#include <algorithm>

#include "../../SessionLayer/SessionLayer.h"
#include "Trains.h"
#include "../../msgTemplates.h"

using namespace std;

Trains::Trains(unique_ptr<CommLayer> commLayer): 
    AlgoLayer(move(commLayer), "fbae.algo.Trains")
{}

void Trains::callbackReceive(string&& serializedMessagePacket) {
    auto trainId{ static_cast<int>(serializedMessagePacket[0]) };
    if (trainId <= nbTrains - 1) {
        processTrain(move(serializedMessagePacket));
    }
    else {
        LOG4CXX_FATAL_FMT(getAlgoLogger(), "Unexpected Train with id {}. Max trains id: {}", trainId, nbTrains - 1);
        exit(EXIT_FAILURE);
    }
}

void Trains::processTrain(string&& serializedMessagePacket) {

    auto train = deserializeStruct<Train>(std::move(serializedMessagePacket));

    if (train.clock < trainsClock[train.id]) {
        LOG4CXX_FATAL_FMT(getAlgoLogger(), "Lost train #{} with clock: {}. Intern clock: {}", train.id, train.clock, trainsClock[train.id]);
        exit(EXIT_FAILURE);
    }

    const rank_t rank = getSessionLayer()->getRank();
    const auto sitesCount = static_cast<uint32_t>(getSessionLayer()->getArguments().getSites().size());
    const rank_t nextRank = (rank + 1) % sitesCount;

    /* DELEVER MESSAGES 
    If it is the same train that brought the batches 
    Then it means it has done a complete loop; so every machine should have received it */
    for (auto const& batch : previousTrainsBatches[train.id]) {
        for (auto const& message : batch.batchSessionMsg) {
            batchNoDeadlockCallbackDeliver(batch.senderPos, message);
        }
    }
    previousTrainsBatches[train.id] = {};

    // Add train's batches to the previous trains' batches vector
    previousTrainsBatches[train.id] = train.batches;

    // Remove train's batches which were sent by next
    auto it = train.batches.begin();
    while (it != train.batches.end()) {
        if (it->senderPos == nextRank) {
            LOG4CXX_INFO_FMT(getAlgoLogger(), "Rank #{:d} removed batch from #{:d} in train {}", rank, nextRank, train.id);
            it = train.batches.erase(it);
            break;
        }
        else { ++it; }
    }

    // Add your own batch of messages to the train and your waiting messages
    if (auto batch{ batchGetBatchMsgs(rank) }; batch.has_value()) {
        train.batches.emplace_back(batch.value());
        previousTrainsBatches[train.id].push_back(batch.value());
        LOG4CXX_INFO_FMT(getAlgoLogger(), "Batch added to train with {} messages", batch.value().batchSessionMsg.size());
    }
    else {
        LOG4CXX_INFO(getAlgoLogger(), "No batch added to train");
    }

    train.clock++;
    trainsClock[train.id] = train.clock;

    const auto serialized = serializeStruct(train);
    getCommLayer()->multicastMsg(serialized);
}

void Trains::execute() {
    const rank_t rank = getSessionLayer()->getRank();
    const auto sitesCount = static_cast<uint32_t>(getSessionLayer()->getArguments().getSites().size());

    previousTrainsBatches = std::vector<std::vector<fbae_AlgoLayer::BatchSessionMsg>>(nbTrains);
    trainsClock = std::vector<int>(nbTrains, 0);

    std::vector<rank_t> broadcasters(sitesCount);
    std::iota(broadcasters.begin(), broadcasters.end(), 0);
    setBroadcastersGroup(std::move(broadcasters));

    LOG4CXX_INFO_FMT(getAlgoLogger(), "Rank: {:d} Next rank: {:d}", rank, (rank + 1) % sitesCount);

    getCommLayer()->openDestAndWaitIncomingMsg({ static_cast<rank_t>((rank + 1) % sitesCount) }, 1, this);
}

int Trains::getClock(int trainId) const { return trainsClock[trainId]; }

string Trains::toString() {
    return "Trains";
}

void Trains::terminate() {
    getCommLayer()->terminate();
}

void Trains::callbackInitDone() {
    AlgoLayer::callbackInitDone();
    trainsClock = vector<int>(nbTrains, 0);

    if (getSessionLayer()->getRank() == 0) {
        for (int i = 0; i < nbTrains; i++) {
            Train train{
                .id = i,
                .clock = 0,
                .batches = {}
            };

            auto serialized = serializeStruct(train);
            callbackReceive(std::move(serialized));
        }
    }
    LOG4CXX_INFO(getAlgoLogger(), "Init Done");
}

void Trains::setNbTrains(int newNbTrains) { nbTrains = newNbTrains; }

vector<vector<fbae_AlgoLayer::BatchSessionMsg >> Trains::getPreviousTrainsBatches() const {
    return previousTrainsBatches;
}

int Trains::getWaitingBatchesNb() const {
    int i = 0;
    for (auto const& batches : previousTrainsBatches) {
        i += batches.size();
    }
    return i;
}

void Trains::addWaitingBatch(int trainId, fbae_AlgoLayer::BatchSessionMsg const& batch) {
    previousTrainsBatches[trainId].push_back(batch);
}
