#pragma once

#include <string>
#include <tuple>
#include <vector>
#include "basicTypes.h"
#include "OptParserExtended.h"
#include "./Logger/LoggerConfig.h"

// The following value has been found experimentally when filler field of SessionPerf has size 0
// (See TEST(SerializationOverhead, CheckMinSizeClientMessageToBroadcast) in @testSerializationOverhead.cpp
constexpr int minSizeClientMessageToBroadcast{31};

// Maximum length of a UDP packet
constexpr size_t maxLength{65515};

constexpr uint16_t defaultNetworkLevelMulticastPort = 30001;

constexpr int specialRankToRequestExecutionInTasks{99};

using HostTuple = std::tuple<std::string, int>;

// Constexpr to access elements of HostTuple
constexpr int HOSTNAME{0};
constexpr int PORT{1};

class Arguments {
private:
    int frequency{0};
    /**
     * @brief True if participant must use network multicast
     */
    int maxBatchSize{INT32_MAX};
    int64_t nbMsg{0};
    std::string networkLevelMulticastAddress;
    uint16_t networkLevelMulticastPort{defaultNetworkLevelMulticastPort};
    rank_t rank{0};
    int sizeMsg{0};
    std::string siteFile{};
    std::vector<HostTuple> sites;
    bool usingNetworkLevelMulticast{false};
    int warmupCooldown{0};
    fbae::LoggerPtr logger{ fbae::getLogger("fbae.arg") };

public:
    explicit Arguments(mlib::OptParserExtended const& parser);
    explicit Arguments(std::vector<HostTuple> const& sites, bool isUsingNetworkLevelMulticast = false); // Constructor used only for tests.
    [[nodiscard]] std::string
    asCsv(std::string const &algoStr, std::string const &commLayerStr, std::string const &rankStr) const;
    static std::string csvHeadline();
    [[nodiscard]] int getFrequency() const;
    [[nodiscard]] int getMaxBatchSize() const;
    [[nodiscard]] int64_t getNbMsg() const;
    [[nodiscard]] std::string_view getNetworkLevelMulticastAddress() const;
    [[nodiscard]] uint16_t getNetworkLevelMulticastPort() const;
    [[nodiscard]] rank_t getRank() const;
    [[nodiscard]] std::vector<HostTuple> getSites() const;
    [[nodiscard]] int getSizeMsg() const;
    [[nodiscard]] int getWarmupCooldown() const;
    [[nodiscard]] bool isUsingNetworkLevelMulticast() const;
};

