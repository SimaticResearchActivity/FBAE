#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"
#include "../AlgoLayer.h"
#include "../../adaptCereal.h"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"

struct Wagon {
    rank_t sender;
    int trainId;
    std::vector<fbae_SessionLayer::SessionMsg> messages;

    template<class Archive> void serialize(Archive& archive) {
        archive(sender, messages, clock);
    }

    friend std::ostream &operator<<(std::ostream &os, Wagon const& wagon) {
        os << "Wagon: " << wagon.sender << "\n";

        for (const auto &message : wagon.messages) {
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
    void terminate() noexcept override;
    void totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) noexcept override;
    void processTrain(std::string&& serializedMessagePacket);

    std::string toString() noexcept override;

private:
    uint32_t clock = 0;

    Wagon wagonToSend{};
    std::vector<Wagon> waitingWagons{};
    std::vector<int> trainsClock{};
};