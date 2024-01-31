#pragma once

#include "../CommPeer.h"
#include "enet/enet.h"

class EnetCommPeer : public CommPeer{
private:
    ENetPeer* enetPeer;
public:
    explicit EnetCommPeer(ENetPeer* enetPeer);
    void disconnect() override;
    void sendMsg(std::string && msg) override;
};
