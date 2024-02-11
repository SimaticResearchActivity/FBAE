#pragma once

#include "../AlgoLayer.h"

class Sequencer : public AlgoLayer {
private:
    /**
     * @brief Rank of sequencer (which is systematically the last participating site).
     */
    rank_t sequencerRank;

public:
    void callbackHandleMessage(std::string && msgString) override;
    void execute() override;
    void totalOrderBroadcast(std::string && msg) override;
    void terminate() override;
    std::string toString() override;
};