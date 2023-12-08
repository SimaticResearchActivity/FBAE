#pragma once

#include "../AlgoLayer.h"

class SequencerAlgoLayer : public AlgoLayer {
private:
    /**
     * @brief Used when SequencerAlgoLayer instance runs as sequencer. Contains number of connected broadcasters.
     */
    int nbConnectedBroadcasters{0};
    /**
     * @brief Used when SequencerAlgoLayer instance runs as a broadcaster. Contains value of next sequence number to deliver.
     */
    int nextDeliver{1};
    /**
     * @brief @CommPeer used to send messages to sequencer.
     */
    std::unique_ptr<CommPeer> sequencerPeer;
    /**
     * @brief Used when SequencerAlgoLayer instance runs as sequencer. Contains last sequence number used by sequencer.
     */
    int seqNum{0};

public:
    bool callbackHandleMessage(std::unique_ptr<CommPeer> peer, const std::string &msgString) override;
    bool executeAndProducedStatistics() override;
    void totalOrderBroadcast(const std::string &msg) override;
    void terminate() override;
    std::string toString() override;
};