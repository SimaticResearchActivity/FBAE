#include <iostream>
#include "AlgoLayer.h"
#include "EnetCommLayer.h"
#include "EnetCommPeer.h"
#include "SessionLayer.h"

constexpr enet_uint32 timeoutFor_enet_host_service{20}; /**< Timeout when calling enet_host_service (this value, defined in milliseconds, was determined by doing experiments with different values) */

using namespace std;

EnetCommLayer::EnetCommLayer() {
    static bool firstCall{true};
    if (firstCall)
    {
        if (enet_initialize() != 0)
        {
            cerr << "ERROR: An error occurred while initializing ENet.\n";
            exit(EXIT_FAILURE);
        }
        atexit(enet_deinitialize);
        firstCall = false;
    }
}

bool EnetCommLayer::analyzeEvent(ENetEvent const& event)
{
    switch (event.type)
    {
        case ENET_EVENT_TYPE_CONNECT:
            if (getAlgoLayer()->getSession()->getParam().getVerbose())
                cout << "\t\tEnetCommLayer : New connection from Host "
                     << showbase << hex << event.peer->address.host << ":" << dec << event.peer->address.port << "\n";
            /* Store any relevant client information in event.peer->data. */
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            if (getAlgoLayer()->getSession()->getParam().getVerbose())
                cout << "\t\tEnetCommLayer : Host "
                     << showbase << hex << event.peer->address.host << ":" << dec << event.peer->address.port
                     << " disconnected.\n";
            /* Reset the peer's client information. */
            event.peer->data = nullptr;
            return true;
        case ENET_EVENT_TYPE_RECEIVE:
            getAlgoLayer()->callbackHandleMessage(
                    make_unique<EnetCommPeer>(event.peer),
                    string{bit_cast<char *>(event.packet->data), event.packet->dataLength});
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_NONE: // Happens when no event occurred within the specified time limit ==> We just ignore this event.
            break;
    }
    return false;
}

void EnetCommLayer::broadcastMsg(std::string_view msg)
{
    ENetPacket* packet = enet_packet_create(msg.data(), msg.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(enetHost, 0, packet); // We send systematically on channelID 0
    // Notice that enet_peer_send() only queues packet as to be sent and, as documentation explains,
    // Queued packets will be sent on a call to enet_host_service(). Alternatively, enet_host_flush()
    // will send out queued packets without dispatching any events.
    // ==> We do enet_host_flush() to be sure that packets are sent immediately.
    enet_host_flush(enetHost);

}

std::unique_ptr<CommPeer> EnetCommLayer::connectToHost(HostTuple host, AlgoLayer *aAlgoLayer)
{
    setAlgoLayer(aAlgoLayer);

    if (!enetHost)
    {
        enetHost = enet_host_create(nullptr, 128, 1, 0, 0); // 128 = 128 outgoing connections, 1 = number of channels
        if (enetHost == nullptr)
        {
            cerr << "ERROR: An error occurred while creating ENet client host.\n";
            exit(EXIT_FAILURE);
        }
    }

    ENetAddress address; // Address of the server
    ENetPeer* peer;  // Contact point to the server
    enet_address_set_host(&address, get<HOSTNAME>(host).c_str());
    address.port = static_cast<enet_uint16>(get<PORT>(host));
    peer = enet_host_connect(enetHost, &address, 1, 0); // Connection with no data sent during connection
    if (peer == nullptr)
    {
        cerr << "ERROR: No available peers for initiating an ENet connection.\n";
        exit(EXIT_FAILURE);
    }
    if (ENetEvent event ; enet_host_service(enetHost, &event, 2000) > 0 && // To check that the server contacted us back. 2000 = Nb milliseconds we want to wait.
                          event.type == ENET_EVENT_TYPE_CONNECT)
    {
        if (getAlgoLayer()->getSession()->getParam().getVerbose())
            cout << "\t\tEnetCommLayer : Connection to server at " << get<HOSTNAME>(host) << ":" << get<PORT>(host) << " succeeded.\n";
    }
    else
    {
        enet_peer_reset(peer);
        cerr << "\t\tEnetCommLayer : Connection to server at " << get<HOSTNAME>(host) << ":" << get<PORT>(host)<< " FAILED (This could be because the server is not running).\n";
        exit(EXIT_FAILURE);
    }

    return make_unique<EnetCommPeer>(peer);
}

void EnetCommLayer::initHost(int port, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer)
{
    setAlgoLayer(aAlgoLayer);

    ENetAddress address{ ENET_HOST_ANY, static_cast<enet_uint16>(port) }; // ENET_HOST_ANY means that anyone can connect to the enetHost
    enetHost = enet_host_create(&address, 128, 1, 0, 0); // 128 = number of clients allowed to connect, 1 = number of channels
    if (enetHost == nullptr)
    {
        cerr << "ERROR: An error occurred while trying to create ENet enetHost host (error could be because there is already a running enetHost bound to port " << port << ").\n";
        exit(EXIT_FAILURE);
    }
}

std::string EnetCommLayer::toString() {
    return "ENet";
}

void EnetCommLayer::waitForMsg(size_t maxDisconnections) {
    size_t remainingDisconnections = maxDisconnections;
    do {
        ENetEvent event;
        while (enet_host_service(enetHost, &event, timeoutFor_enet_host_service) > 0)
            if (analyzeEvent(event))
                --remainingDisconnections;

    } while (remainingDisconnections);
}
