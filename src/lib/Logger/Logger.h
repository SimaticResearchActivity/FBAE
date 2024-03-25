#pragma once

#include <string>

class Logger {
public:
    void LogInfo(std::string &caller, std::string &message);
    void LogError(std::string &caller, std::string &message);
    void LogDebug(std::string &caller, std::string &message);
};

