#pragma once

#include "../../SessionLayer/SessionLayerMsg.h"
#include "../AlgoLayer.h"

struct Wagon {
    int sender;
    int rotation;
    std::vector<fbae_SessionLayer::SessionMsg> msgs;
};

struct Train {
    int id;
    int clock;
    int rotation;
    std::vector<Wagon> wagons;
    bool initialize = true;

    template<class Archive> void serialize(Archive& archive) {
        archive(id,clock,rotation,wagons);
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
    // Mettre tout ce dont on a besoin : fonctions + paramètres de notre classe

    void UTODeliver(std::vector<fbae_SessionLayer::SessionMsg> messages, std::vector<rank_t> ranks);

    static const int DELAY;
    static const int NB_ROT = 3;
    static const int NB_TR = 2;
    int idLast;
    bool initDone;
    Train lastTrains[NB_TR];
    int nbJoin;
    std::vector<Wagon> receivedWagons[NB_TR][NB_ROT];
    Wagon wagonToSend;

};

