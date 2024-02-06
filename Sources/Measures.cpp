#include <algorithm>
#include <cassert>
#include <numeric>
#include "Measures.h"

Measures::Measures(size_t nbPingMax)
: pings(nbPingMax)
{
}

void Measures::add(std::chrono::duration<double, std::milli> const& elapsed)
{
    size_t index{nbPing++};
    assert(index < pings.size());
    pings[index] = elapsed;
}

void Measures::addNbBytesDelivered(const int nb)
{
    if (measuresUndergoing)
        nbBytesDelivered += nb;
}

std::string Measures::csvHeadline()
{
    return std::string { "nbPing,Average (in ms),Min,Q(0.25),Q(0.5),Q(0.75),Q(0.99),Q(0.999),Q(0.9999),Max,Elapsed time (in sec),CPU time (in sec),Throughput (in Mbps)"};
}

std::string Measures::asCsv()
{
    pings.resize(nbPing);
    std::ranges::sort(pings);

    // Predefined units are nanoseconds, microseconds, milliseconds, seconds, minutes, hours.
    // See https://www.geeksforgeeks.org/measure-execution-time-function-cpp/
    using namespace std::chrono;
    const auto duration = duration_cast<milliseconds>(stopTime - startTime);

    constexpr int nbBitsPerByte{ 8 };
    constexpr int nbBitsPerMega{ 1'000'000 };
    constexpr double nbMillisecondsPerSecond{ 1'000.0};
    constexpr double nbMicrosecondsPerSecond{ 1'000'000.0};

    auto mbps = static_cast<double>(nbBytesDelivered * nbBitsPerByte)
                       / (static_cast<double>(duration.count()) / nbMillisecondsPerSecond)
                       / nbBitsPerMega;

    return std::string {
            std::to_string(pings.size()) + ","
            + std::to_string((std::reduce(pings.begin(), pings.end()) / pings.size()).count()) + ","
            + std::to_string(pings[0].count()) + ","
            + std::to_string(pings[pings.size() / 4].count()) + ","
            + std::to_string(pings[pings.size() / 2].count()) + ","
            + std::to_string(pings[pings.size() * 3 / 4].count()) + ","
            + std::to_string(pings[pings.size() * 99 / 100].count()) + ","
            + std::to_string(pings[pings.size() * 999 / 1'000].count()) + ","
            + std::to_string(pings[pings.size() * 9999 / 10'000].count()) + ","
            + std::to_string(pings[pings.size() - 1].count()) + ","
            + std::to_string(static_cast<double>(duration.count()) / nbMillisecondsPerSecond) + ","
            + std::to_string(static_cast<double>(stopTimeCpu - startTimeCpu) / nbMicrosecondsPerSecond) + ","
            + std::to_string(mbps)
    };
}

void Measures::setStartTime() {
    startTime = std::chrono::system_clock::now();
    startTimeCpu = get_cpu_time();
    measuresUndergoing = true;
}

void Measures::setStopTime() {
    stopTime = std::chrono::system_clock::now();
    stopTimeCpu = get_cpu_time();
    measuresUndergoing = true;
}
