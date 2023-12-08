#pragma once

#include <string>

class CommPeer {
public:
    virtual ~CommPeer() = default;
    /**
     * @brief Disconnects from peer.
     */
    virtual void disconnect() = 0;
    /**
     * @brief Sends @msg to peer.
     * @param msg Message to send.
     */
    virtual void sendMsg(std::string_view msg) = 0;
};
