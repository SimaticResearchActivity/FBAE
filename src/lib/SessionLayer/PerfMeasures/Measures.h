#pragma once
#include <atomic>
#include <chrono>
#include <string>
#include <vector>

#include "get_cpu_time.h"

class Measures {
 public:
  explicit Measures(size_t nbPingMax);
  void add(std::chrono::duration<double, std::milli> const &elapsed);
  void addNbBytesDelivered(int nb);
  std::string asCsv();
  static std::string csvHeadline();
  void setStartTime();
  void setStopTime();

 private:
  std::vector<std::chrono::duration<double, std::milli>>
      pings;  // Round-Trip Time
  std::atomic_size_t nbPing{0};
  int64_t nbBytesDelivered{0};
  bool measuresUndergoing{false};
  std::chrono::time_point<std::chrono::system_clock> startTime;
  std::chrono::time_point<std::chrono::system_clock> stopTime;
  unsigned long long startTimeCpu{0};
  unsigned long long stopTimeCpu{0};
};