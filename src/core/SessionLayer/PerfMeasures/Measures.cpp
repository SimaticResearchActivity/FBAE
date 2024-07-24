#include "Measures.h"

#include <algorithm>
#include <cassert>
#include <numeric>

namespace fbae::core::SessionLayer::PerfMeasures {

Measures::Measures(size_t nbPingMax) : pings(nbPingMax) {
  if (string errmsg; YAPI::RegisterHub("usb", errmsg) != YAPI::SUCCESS) {
    LOG4CXX_ERROR_FMT(m_logger, "RegisterHub error: {}", errmsg);
    wattMeterAvailable = false;
  } else {
    if (wattMeter = YPower::FirstPower(); wattMeter == nullptr) {
      LOG4CXX_WARN(m_logger, "Could not find wattmeter");
      wattMeterAvailable = false;
    }
  }

  int intTest = 2;
  int* test = &intTest;

  perfmon_init(1, test);
}

void Measures::add(std::chrono::duration<double, std::milli> const& elapsed) {
  size_t index{nbPing++};
  assert(index < pings.size());
  pings[index] = elapsed;
}

void Measures::addNbBytesDelivered(const int nb) {
  if (measuresUndergoing) nbBytesDelivered += nb;
}

std::string Measures::csvHeadline() {
  return std::string{
      "nbPing,Average (in "
      "ms),Min,Q(0.25),Q(0.5),Q(0.75),Q(0.99),Q(0.999),Q(0.9999),Max,Elapsed "
      "time (in sec),CPU time (in sec),Throughput (in Mbps),Energy Delivered (in Wh)"};
}

std::string Measures::asCsv() {
  pings.resize(nbPing);
  std::ranges::sort(pings);

  // Predefined units are nanoseconds, microseconds, milliseconds, seconds,
  // minutes, hours. See
  // https://www.geeksforgeeks.org/measure-execution-time-function-cpp/
  using namespace std::chrono;
  const auto duration = duration_cast<milliseconds>(stopTime - startTime);

  constexpr int nbBitsPerByte{8};
  constexpr int nbBitsPerMega{1'000'000};
  constexpr double nbMillisecondsPerSecond{1'000.0};
  constexpr double nbMicrosecondsPerSecond{1'000'000.0};

  auto mbps =
      static_cast<double>(nbBytesDelivered * nbBitsPerByte) /
      (static_cast<double>(duration.count()) / nbMillisecondsPerSecond) /
      nbBitsPerMega;

  string deliveredEnergyStr = "Non Available";
  if (wattMeterAvailable) {
    std::ostringstream strs;
    strs << deliveredEnergy;
    deliveredEnergyStr = strs.str();
  }

  return std::format(
      "{},{},{},{},{},{},{},{},{},{},{},{},{},{}", pings.size(),
      (std::reduce(pings.begin(), pings.end()) / pings.size()).count(),
      pings[0].count(), pings[pings.size() / 4].count(),
      pings[pings.size() / 2].count(), pings[pings.size() * 3 / 4].count(),
      pings[pings.size() * 99 / 100].count(),
      pings[pings.size() * 999 / 1'000].count(),
      pings[pings.size() * 9999 / 10'000].count(),
      pings[pings.size() - 1].count(),
      static_cast<double>(duration.count()) / nbMillisecondsPerSecond,
      static_cast<double>(stopTimeCpu - startTimeCpu) / nbMicrosecondsPerSecond,
      mbps,
      deliveredEnergyStr);
}

void Measures::setStartTime() {
  if (wattMeterAvailable && wattMeter->isOnline()) {
    LOG4CXX_INFO(m_logger, "Reset wattmeter");
    wattMeter->reset();
  }
  else {
    LOG4CXX_WARN(m_logger, "Could not do the Yoctopuce's reset");
    wattMeterAvailable = false;
  }

  startTime = std::chrono::system_clock::now();
  startTimeCpu = get_cpu_time();
  measuresUndergoing = true;
}

void Measures::setStopTime() {
  stopTime = std::chrono::system_clock::now();
  stopTimeCpu = get_cpu_time();

  if (wattMeterAvailable && wattMeter->isOnline()) {
    deliveredEnergy = wattMeter->get_deliveredEnergyMeter();
    LOG4CXX_INFO_FMT(m_logger, "Energy delivered: {}", deliveredEnergy);
  }
  else {
    LOG4CXX_WARN(m_logger, "Could not do the mesure with Yoctopuce");
    wattMeterAvailable = false;
  }
  measuresUndergoing = true;
}

}  // namespace fbae::core::SessionLayer::PerfMeasures
