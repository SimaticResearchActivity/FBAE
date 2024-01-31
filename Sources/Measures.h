#pragma once
#include <atomic>
#include <chrono>
#include <string>
#include <vector>

class Measures {
public:
    explicit Measures(size_t nbPingMax);
    void add(std::chrono::duration<double, std::milli> const &elapsed);
    std::string asCsv(int argSizeMsg);
    static std::string csvHeadline();
    void setStartTime();
    void setStopTime();
private:
    std::vector<std::chrono::duration<double, std::milli>> pings; // Round-Trip Time
    std::atomic_size_t nbPing{0};
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::chrono::time_point<std::chrono::system_clock> stopTime;
};