#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"
#include "../AlgoLayer.h"
#include "../../adaptCereal.h"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"

struct TrainMessage {
    fbae_SessionLayer::SessionMsg message;
    uint32_t clock;

    template<class Archive> void serialize(Archive& archive) {
        archive(message, clock);
    }
};

struct Wagon {
    int sender;
    std::vector<TrainMessage> msgs;
    uint32_t clock;

    template<class Archive> void serialize(Archive& archive) {
        archive(sender, msgs, clock);
    }

    friend std::ostream &operator<<(std::ostream &os, Wagon wagon) {
        os << "Wagon: " << wagon.sender << ", clock: " << wagon.clock << "\n";

        for (const auto &msg : wagon.msgs) {
            os << static_cast<int>(msg.message->msgId) << "\t";
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
    explicit Trains(std::unique_ptr<CommLayer> commLayer) noexcept;

    void callbackReceive(std::string && serializedMessagePacket) noexcept override;
    void execute() noexcept override;
    void terminate() noexcept override;
    void totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) noexcept override;

    std::string toString() noexcept override;

private:
    std::vector<Wagon> pendingWagons;
    Wagon wagonToSend;
    uint32_t clock;
};