#pragma once

#include <string>

class Logger {
public:
    explicit Logger();

    void logTrace(const std::string && caller, const std::string && message);
    void logInfo(const std::string && caller, const std::string && message);
    void logError(const std::string && caller, const std::string && message);
    void logDebug(const std::string && caller, const std::string && message);
    void logWarn(const std::string && caller, const std::string && message);
    void logFatal(const std::string && caller, const std::string && message);
};

void initializeLogger();
const Logger &getLogger();
