#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"
#include "../AlgoLayer.h"
#include "../../adaptCereal.h"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"

struct Wagon {
    int trainId;
    fbae_AlgoLayer::BatchSessionMsg batch;

    template<class Archive> void serialize(Archive& archive) {
        archive(trainId, batch);
    }

    friend std::ostream &operator<<(std::ostream &os, Wagon const& wagon) {
        os << "Wagon: " << wagon.batch.senderPos << "\n";

        for (const auto &message : wagon.batch.batchSessionMsg) {
            os << static_cast<int>(message->msgId) << "\t";
        }
        os << "\n";
        return os;
    }
};

struct Train {
    int id;
    int clock;
    std::vector<Wagon> wagons;

    template<class Archive> void serialize(Archive& archive) {
        archive(id, clock, wagons);
    }
};

class Trains: public AlgoLayer {
public:
    explicit Trains(std::unique_ptr<CommLayer> commLayer);

    void callbackReceive(std::string && serializedMessagePacket) override;
    void execute() override;
    void terminate() override;
    void totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) override;
    void processTrain(std::string&& serializedMessagePacket);

    std::string toString() override;

private:
    Wagon wagonToSend{};
    std::vector<Wagon> waitingWagons{};
    std::vector<int> trainsClock{};
};