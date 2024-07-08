#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"
#include "../AlgoLayer.h"
#include "../../adaptCereal.h"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"


struct Train {
    int id;
    int clock;
    std::vector<fbae_AlgoLayer::BatchSessionMsg> batches;

    template<class Archive> void serialize(Archive& archive) {
        archive(id, clock, batches);
    }
};

class Trains: public AlgoLayer {
public:
    explicit Trains(std::unique_ptr<CommLayer> commLayer);

    void callbackReceive(std::string && serializedMessagePacket) override;
    void execute() override;
    void terminate() override;
    void processTrain(std::string&& serializedMessagePacket);
    int getClock(int trainId) const;
    void callbackInitDone() override;
    std::string toString() override;

    void setNbTrains(int newNbTrains);
    std::vector<std::vector<fbae_AlgoLayer::BatchSessionMsg>> getPreviousTrainsBatches() const;
    int getWaitingBatchesNb() const;
    void addWaitingBatch(int trainId, fbae_AlgoLayer::BatchSessionMsg const& batch);

private:
    /**
    * @brief Number of trains
     */
    int nbTrains{ 3 };

    /**
     * @brief Vector to keep the batches brought by the trains 
     */
    std::vector<std::vector<fbae_AlgoLayer::BatchSessionMsg>> previousTrainsBatches = std::vector<std::vector<fbae_AlgoLayer::BatchSessionMsg>>(nbTrains);

    /**
    * @brief Logical clocks of the trains
     */
    std::vector<int> trainsClock = std::vector<int>(nbTrains, 0);

    /**
    * @brief Rank of machine
    */
    rank_t rank;

    /**
    * @brief Number of machines
    */
    uint32_t sitesCount;

    /**
    * @brief Rank of next machine in loop
    */
    rank_t nextRank;

    /**
    * @brief True when @disconnect() method has been called.
    */
    bool algoTerminated{ false };
};