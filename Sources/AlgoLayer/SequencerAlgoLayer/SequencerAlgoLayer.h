#pragma once

#include "../AlgoLayer.h"

class SequencerAlgoLayer : public AlgoLayer {
private:
    /**
     * @brief Rank of sequencer.
     */
    rank_t sequencerRank;

public:
    void callbackHandleMessage(std::string && msgString) override;
    bool executeAndCheckIfProducedStatistics() override;
    void totalOrderBroadcast(std::string && msg) override;
    void terminate() override;
    std::string toString() override;
};