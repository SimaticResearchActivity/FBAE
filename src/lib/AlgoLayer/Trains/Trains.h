#pragma once

#include "../AlgoLayer.h"

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

};

