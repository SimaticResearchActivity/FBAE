#include "Trains.h"

#include <algorithm>
#include <iostream>
#include <numeric>

#include "../../SessionLayer/SessionLayer.h"
#include "../../msgTemplates.h"

using namespace std;

namespace fbae::core::AlgoLayer::Trains {

Trains::Trains(unique_ptr<fbae::core::CommLayer::CommLayer> commLayer)
    : AlgoLayer(std::move(commLayer), "fbae.algo.Trains") {}

void Trains::callbackReceive(string&& serializedMessagePacket) {
  if (algoTerminated) {
    return;
  }

  char trainId{serializedMessagePacket[0]};
  char trainClock{serializedMessagePacket[1]};
  LOG4CXX_INFO_FMT(getAlgoLogger(), "Rank #{:d}: Train #{:d} with clock {:d}",
                   rank, trainId, trainClock);

  if (trainId > nbTrains - 1) {
    LOG4CXX_FATAL_FMT(getAlgoLogger(),
                      "Unexpected Train #{:d}. Max trains id: {}", trainId,
                      nbTrains - 1);
    exit(EXIT_FAILURE);
  }
  if (isTrainRecent(trainId, trainClock)) {
    processTrain(std::move(serializedMessagePacket));
  } else {
    LOG4CXX_FATAL_FMT(
        getAlgoLogger(),
        "Rank #{:d}: Unexpected train #{:d} with clock {:d}. Algo clock: {}",
        rank, trainId, trainClock, trainsClock[trainId]);
    exit(EXIT_FAILURE);
  }
}

void Trains::processTrain(string&& serializedMessagePacket) {
  auto train = deserializeStruct<Train>(std::move(serializedMessagePacket));

  /* DELEVER MESSAGES
  If it is the same train that brought the batches
  Then it means it has done a complete loop; so every machine should have
  received it */
  vector<BatchSessionMsg> batchesToDeliver =
      previousTrainsBatches[train.id];
  previousTrainsBatches[train.id].clear();

  // Add train's batches to the previous trains' batches vector
  previousTrainsBatches[train.id] = train.batches;

  // Remove train's batches which were sent by next
  auto it = train.batches.begin();
  while (it != train.batches.end()) {
    if (it->senderPos == nextRank) {
      LOG4CXX_INFO_FMT(getAlgoLogger(),
                       "Rank #{:d} removed batch from #{:d} in train {}", rank,
                       nextRank, train.id);
      it = train.batches.erase(it);
      break;
    } else {
      ++it;
    }
  }

  // Add your own batch of messages to the train and your waiting messages
  if (auto batch{batchGetBatchMsgs(rank)}; batch.has_value()) {
    train.batches.emplace_back(batch.value());
    previousTrainsBatches[train.id].push_back(batch.value());
    LOG4CXX_INFO_FMT(getAlgoLogger(), "Batch added to train with {} messages",
                     batch.value().batchSessionMsg.size());
  } else {
    LOG4CXX_INFO(getAlgoLogger(), "No batch added to train");
  }

  train.clock++;
  trainsClock[train.id] = train.clock;
  lastTrainId = train.id;

  if (!algoTerminated) {
    const auto serialized = serializeStruct<Train>(train);
    getCommLayer()->send(nextRank, serialized);
  }

  // Deliver the messages that should be delivered
  for (auto const& batch : batchesToDeliver) {
    for (auto const& message : batch.batchSessionMsg) {
      batchNoDeadlockCallbackDeliver(batch.senderPos, message);
    }
  }
}

void Trains::execute() {
  rank = getSessionLayer()->getRank();
  sitesCount = static_cast<uint32_t>(
      getSessionLayer()->getArguments().getSites().size());
  nextRank = (rank + 1) % sitesCount;

  nbTrains = getSessionLayer()->getArguments().getIntInAlgoArgument("trainsNb",
                                                                    nbTrains);
  if (const short N = 128; nbTrains > N) {
    LOG4CXX_FATAL_FMT(getAlgoLogger(),
                      "Number of train ({}) should be inferior to {}", nbTrains,
                      N);
    exit(EXIT_FAILURE);
  }

  previousTrainsBatches =
      std::vector<std::vector<BatchSessionMsg>>(nbTrains);
  trainsClock = std::vector<uint8_t>(nbTrains, 0);

  std::vector<rank_t> broadcasters(sitesCount);
  std::iota(broadcasters.begin(), broadcasters.end(), 0);
  setBroadcastersGroup(std::move(broadcasters));

  LOG4CXX_INFO_FMT(getAlgoLogger(), "Rank #{:d}; Next rank #{:d}", rank,
                   nextRank);

  getCommLayer()->openDestAndWaitIncomingMsg({nextRank}, 1, this);
}

int Trains::getClock(int trainId) const { return trainsClock[trainId]; }

string Trains::toString() { return "Trains"; }

void Trains::terminate() {
  algoTerminated = true;
  getCommLayer()->terminate();
}

void Trains::callbackInitDone() {
  AlgoLayer::callbackInitDone();

  if (getSessionLayer()->getRank() == 0) {
    for (uint8_t i = 0; i < nbTrains; i++) {
      Train train{.id = i, .clock = 1, .batches = {}};

      auto serialized = serializeStruct<Train>(train);
      callbackReceive(std::move(serialized));
    }
  }
  LOG4CXX_INFO(getAlgoLogger(), "Init Done");
}

bool Trains::isTrainRecent(uint8_t trainId, uint8_t trainClock) {
  const int16_t M =
      256;  // See explanation of M in section 6.1.1.2 at page 123 of PhD thesis
            // https://theses.hal.science/tel-00787598
  auto idExpected = static_cast<short>((lastTrainId + 1) % nbTrains);

  if (trainId == idExpected) {
    uint8_t diff = trainClock - trainsClock[trainId];

    if (diff > 0) {
      return cmp_less(diff, ((1 + M) / 2));
    } else {
      return cmp_less(diff, ((1 - M) / 2));
    }
  } else {
    LOG4CXX_FATAL_FMT(getAlgoLogger(),
                      "Rank #{:d}: Train expected: {:d}, Train got: {:d}", rank,
                      idExpected, trainId);
    return false;
  }
}

void Trains::setNbTrains(int newNbTrains) { nbTrains = newNbTrains; }

vector<vector<fbae::core::AlgoLayer::BatchSessionMsg>>
Trains::getPreviousTrainsBatches() const {
  return previousTrainsBatches;
}

int Trains::getWaitingBatchesNb() const {
  int i = 0;
  for (auto const& batches : previousTrainsBatches) {
    i += batches.size();
  }
  return i;
}

void Trains::addWaitingBatch(int trainId,
                             BatchSessionMsg const& batch) {
  previousTrainsBatches[trainId].push_back(batch);
}

}  // namespace fbae::core::AlgoLayer::Trains
