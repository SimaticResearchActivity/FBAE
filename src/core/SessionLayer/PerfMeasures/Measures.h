#pragma once
#include <atomic>
#include <chrono>
#include <string>
#include <vector>

#include "Logger/LoggerConfig.h"
#include "get_cpu_time.h"
#include "yocto_api.h"
#include "yocto_power.h"

namespace fbae::core::SessionLayer::PerfMeasures {

class Measures {
 public:
  explicit Measures(size_t nbPingMax, std::string  externalMeasureLabel = "");
  void add(std::chrono::duration<double, std::milli> const &elapsed);
  void addNbBytesDelivered(int nb);
  std::string asCsv();
  [[nodiscard]] std::string asCsvCaliber() const;
  void yoctoMeterMeasuresAsCsv(std::string &deliveredEnergyStr,
    std::string &externalMeasureLabelStr, std::string &externalMeasureValueStr) const;
  static std::string csvHeadline();
  static std::string csvCaliberHeadline();
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

  YPower* wattMeter = nullptr;
  double deliveredEnergy = -1;
  bool wattMeterAvailable = true;

  std::string externalMeasureLabel;

  fbae::core::Logger::LoggerPtr m_logger = fbae::core::Logger::getLogger("fbae.core.SessionLayer.Measures");
};

}  // namespace fbae::core::SessionLayer::PerfMeasures
