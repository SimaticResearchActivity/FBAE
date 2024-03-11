#pragma once

#include "../AlgoLayer.h"
#include "LCRMessage.h"

/** The LCR Algorithm layer. */
class LCRLayer: AlgoLayer {
public:
    explicit LCRLayer(std::unique_ptr<CommLayer> commLayer);
    void callbackReceive(std::string && algoMsgAsString) override;
    void execute() override;
    void terminate() override;
    void totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) override;
    std::string toString() override;
private:
    // Things are meant to go to done in each process, but instead I'll just put it
    // all in the algo layer?
    /** The data each process must have. */
    struct ProcessData {
    public:
        /** The vector clock of the process. */
        std::vector<uint32_t> vectorClock;
        /** The pending list of messages to be delivered to this process. */
        std::vector<fbae_LCRAlgoLayer::StructBroadcastMessage> pending;

        explicit ProcessData(rank_t processCount);
    };
    /** The data for each process */
    std::vector<ProcessData> processData;
};

