#pragma once

#include <string>
#include <tuple>
#include <vector>
#include "OptParserExtended.h"

// The following value has been found experimentally when filler field of SenderMessageToBroadcast has size 0
constexpr int minSizeClientMessageToBroadcast{22};

// Maximum length of a UDP packet
constexpr size_t maxLength{65515};

constexpr int specialRankToRequestExecutionInThreads{99};

using HostTuple = std::tuple<std::string, int>;

// Constexpr to access elements of HostTuple
constexpr int HOSTNAME{0};
constexpr int PORT{1};

class Param {
private:
    int frequency{0};
    int maxBatchSize{INT32_MAX};
    int nbMsg{0};
    uint8_t rank{0};
    int sizeMsg{0};
    std::string siteFile{};
    std::vector<HostTuple> sites;
    bool verbose{false};

public:
    explicit Param(mlib::OptParserExtended const& parser);
    [[nodiscard]] std::string asCsv(std::string const& algoStr, std::string const& commLayerStr) const;
    static std::string csvHeadline();
    [[nodiscard]] int getFrequency() const;
    [[nodiscard]] int getMaxBatchSize() const;
    [[nodiscard]] int getNbMsg() const;
    [[nodiscard]] uint8_t getRank() const;
    [[nodiscard]] std::vector<HostTuple> getSites() const;
    [[nodiscard]] int getSizeMsg() const;
    [[nodiscard]] bool getVerbose() const;
};

