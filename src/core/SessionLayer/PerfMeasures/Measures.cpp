#include "Measures.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>

namespace fbae::core::SessionLayer::PerfMeasures {

Measures::Measures(size_t const nbPingMax, std::string externalMeasureLabel)
: pings(nbPingMax)
, externalMeasureLabel{std::move(externalMeasureLabel)}
{
  if (string errmsg; YAPI::RegisterHub("usb", errmsg) != YAPI::SUCCESS) {
    LOG4CXX_WARN_FMT(m_logger, "RegisterHub error: {}; Trying by VirtualHub", errmsg);

    if (YAPI::RegisterHub("localhost:4444", errmsg) != YAPI::SUCCESS) {
      LOG4CXX_WARN_FMT(m_logger, "VirtualHub RegisterHub error: {};", errmsg);
      wattMeterAvailable = false;
    }
  }

  if (wattMeterAvailable) {
    if (wattMeter = YPower::FirstPower(); wattMeter == nullptr) {
      LOG4CXX_WARN(m_logger, "Could not find watt meter");
      wattMeterAvailable = false;
    }
  }
}

void Measures::add(std::chrono::duration<double, std::milli> const& elapsed) {
  size_t const index{nbPing++};
  assert(index < pings.size());
  pings[index] = elapsed;
}

void Measures::addNbBytesDelivered(const int nb) {
  if (measuresUndergoing) nbBytesDelivered += nb;
}

std::string Measures::csvHeadline() {
  return std::string{"nbPing,Average (in ms),Min,Q(0.25),Q(0.5),Q(0.75),Q(0.99),Q(0.999),Q(0.9999),Max,"
  "Elapsed time (in sec),CPU time (in sec),Throughput (in Mbps),Energy Delivered (in Wh),External Measure, External Measure Value (in Wh)"};
}

std::string Measures::csvCaliberHeadline() {
  return std::string{"Calibration Elapsed time (in sec),Calibration CPU time (in sec),Calibration Throughput (in Mbps),Calibration Energy Delivered (in Wh),Calibration External Measure,Calibration External Measure Value (in Wh)"};
}

std::string Measures::asCsv() {
  pings.resize(nbPing);
  std::ranges::sort(pings);

  return std::format(
      "{},{},{},{},{},{},{},{},{},{},{}", pings.size(),
      (std::reduce(pings.begin(), pings.end()) / pings.size()).count(),
      pings[0].count(), pings[pings.size() / 4].count(),
      pings[pings.size() / 2].count(), pings[pings.size() * 3 / 4].count(),
      pings[pings.size() * 99 / 100].count(),
      pings[pings.size() * 999 / 1'000].count(),
      pings[pings.size() * 9999 / 10'000].count(),
      pings[pings.size() - 1].count(), 
      asCsvCaliber());
}

std::string Measures::asCsvCaliber() const {
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

  string deliveredEnergyStr;
  string externalMeasureLabelStr;
  string externalMeasureValueStr;

  yoctoMeterMeasuresAsCsv(deliveredEnergyStr, externalMeasureLabelStr,
    externalMeasureValueStr);

  return std::format(
      "{},{},{},{},{},{}",
      static_cast<double>(duration.count()) / nbMillisecondsPerSecond,
      static_cast<double>(stopTimeCpu - startTimeCpu) / nbMicrosecondsPerSecond,
      mbps,
      deliveredEnergyStr,
      externalMeasureLabelStr,
      externalMeasureValueStr);
}

void Measures::yoctoMeterMeasuresAsCsv(
    std::string &deliveredEnergyStr,
  std::string &externalMeasureLabelStr, std::string &externalMeasureValueStr) const {
  deliveredEnergyStr = "Non Available";
  externalMeasureLabelStr = "-";
  externalMeasureValueStr = "-";

  if (wattMeterAvailable) {
    std::ostringstream strStream;
    strStream << deliveredEnergy;
    deliveredEnergyStr = strStream.str();
  }

  if (!externalMeasureLabel.empty()) {
    externalMeasureLabelStr = externalMeasureLabel;
    externalMeasureValueStr = deliveredEnergyStr;
    deliveredEnergyStr = "-";
  }
}

void Measures::setStartTime() {
  // Measures with yocto meter
  if (wattMeterAvailable) {
    if (wattMeter->isOnline()) {
      LOG4CXX_INFO(m_logger, "Reset watt meter");
      wattMeter->reset();
    }
    else {
      LOG4CXX_WARN(m_logger, "Could not do the Yocto meter's reset");
      wattMeterAvailable = false;
    }
  }

  startTime = std::chrono::system_clock::now();
  startTimeCpu = get_cpu_time();
  measuresUndergoing = true;
}

void Measures::setStopTime() {
  stopTime = std::chrono::system_clock::now();
  stopTimeCpu = get_cpu_time();

  if (wattMeterAvailable) {
    if (wattMeter->isOnline()) {
      deliveredEnergy = wattMeter->get_deliveredEnergyMeter();
      LOG4CXX_INFO_FMT(m_logger, "Energy delivered: {}", deliveredEnergy);
    }
    else {
      LOG4CXX_WARN(m_logger, "Could not do the measure with yocto meter");
      wattMeterAvailable = false;
    }
  }

  measuresUndergoing = true;
}

}  // namespace fbae::core::SessionLayer::PerfMeasures
