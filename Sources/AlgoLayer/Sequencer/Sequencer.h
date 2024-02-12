#pragma once

#include "../AlgoLayer.h"

/**
 * @brief Rank of sequencer (which is systematically 0).
 */
constexpr static rank_t sequencerRank{0};

class Sequencer : public AlgoLayer {
public:
    void callbackHandleMessage(std::string && msgString) override;
    void execute() override;
    void totalOrderBroadcast(std::string && msg) override;
    void terminate() override;
    std::string toString() override;
};