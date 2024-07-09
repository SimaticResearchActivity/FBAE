#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "./Logger/LoggerConfig.h"
#include "OptParserExtended.h"
#include "basicTypes.h"

namespace fbae::core {

// The following value has been found experimentally when filler field of
// SessionPerf has size 0 (See TEST(SerializationOverhead,
// CheckMinSizeClientMessageToBroadcast) in @testSerializationOverhead.cpp
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
  std::string algoArgument;
  std::string commArgument;
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
  fbae::core::Logger::LoggerPtr logger{fbae::core::Logger::getLogger("fbae.core.arg")};

  /**
   * @brief Searches @searchedArgument in @arg.
   * @param searchedArgument Searched argument
   * @param defaultIntValue Value to be returned if @searchedArgument is not
   * found
   * @param arg Argument where to search @searchedArgument
   * @param argName Name for @arg
   * @return Argument value corresponding to @searchedArgument or
   * @defaultIntValue if it is not found
   */
  [[nodiscard]] int genericGetIntInArgument(std::string_view searchedArgument,
                                            int defaultIntValue,
                                            std::string_view arg,
                                            std::string_view argName) const;

 public:
  explicit Arguments(fbae::core::OptParserExtended const &parser);
  explicit Arguments(std::vector<HostTuple> const &sites,
                     std::string_view algoArgument = "",
                     std::string_view commArgument = "",
                     bool isUsingNetworkLevelMulticast =
                         false);  // Constructor used only for tests.
  [[nodiscard]] std::string asCsv(std::string const &algoStr,
                                  std::string const &commLayerStr,
                                  std::string const &rankStr) const;
  static std::string csvHeadline();
  [[nodiscard]] int getFrequency() const;
  /**
   * @brief Searches @searchedArgument in @algoArgument.
   * @param searchedArgument Searched argument
   * @param defaultIntValue Value to be returned if @searchedArgument is not
   * found
   * @return Argument value corresponding to @searchedArgument or
   * @defaultIntValue if it is not found
   */
  [[nodiscard]] int getIntInAlgoArgument(std::string_view searchedArgument,
                                         int defaultIntValue) const;
  /**
   * @brief Searches @searchedArgument in @commArgument.
   * @param searchedArgument Searched argument
   * @param defaultIntValue Value to be returned if @searchedArgument is not
   * found
   * @return Argument value corresponding to @searchedArgument or
   * @defaultIntValue if it is not found
   */
  [[nodiscard]] int getIntInCommArgument(std::string_view searchedArgument,
                                         int defaultIntValue) const;
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

}  // namespace fbae::core
