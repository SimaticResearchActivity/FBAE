#pragma once

#include "../AlgoLayer.h"

class SequencerAlgoLayer : public AlgoLayer {
private:
    /**
     * @brief Used when SequencerAlgoLayer instance runs as sequencer. Contains number of connected broadcasters.
     */
    int nbConnectedBroadcasters{0};

    /**
     * @brief @CommPeer used to send messages to sequencer.
     */
    std::unique_ptr<CommPeer> sequencerPeer;
    int rank;
    int size;
    bool receive = true;

public:
    bool callbackHandleMessage(std::unique_ptr<CommPeer> peer, std::string && msgString) override;
    bool executeAndProducedStatistics() override;
    void totalOrderBroadcast(std::string && msg) override;
    void terminate() override;
    std::string toString() override;
};