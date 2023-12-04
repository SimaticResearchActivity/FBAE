#pragma once

#include <enet/enet.h>
#include "CommLayer.h"
#include "Param.h"

class EnetCommLayer : public CommLayer{
private:
    /**
     * @brief Used to store the result of enet_host_create when process is a host or a plain client
     */
    ENetHost* enetHost{nullptr};
    /**
     * @brief Analyze event @event received as an Enet host.
     * @param event Event to be analyzed
     * @param nonHostPeer Call @algoLayer->@handleMessageAsNonHostPeer() if @nonHostPeer is true and
     * @algoLayer->@handleMessageAsHost() otherwise.
     * @return true if the received event is a disconnection and false otherwise.
     */
    bool analyzeEvent(ENetEvent const& event);
public:
    EnetCommLayer();
    void broadcastMsg(std::string_view msg) override;
    std::unique_ptr<CommPeer> connectToHost(HostTuple host, AlgoLayer *algoLayer) override;
    void initHost(int port, size_t nbAwaitedConnections, AlgoLayer *aAlgoLayer) override;
    std::string toString() override;
    void waitForMsg(size_t maxDisconnections) override;
};
