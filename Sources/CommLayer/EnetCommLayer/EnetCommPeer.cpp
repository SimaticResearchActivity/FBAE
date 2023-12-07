#include "EnetCommPeer.h"

EnetCommPeer::EnetCommPeer(ENetPeer *enetPeer)
        : enetPeer{enetPeer}
{
}

void EnetCommPeer::disconnect() {
    enet_peer_disconnect(enetPeer, 0); // Instead of 0, we could have sent some data
}

void EnetCommPeer::sendMsg(std::string_view msg) {
    ENetPacket* packet = enet_packet_create(msg.data(), msg.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(enetPeer, 0, packet); // We send systematically on channelID 0
}
