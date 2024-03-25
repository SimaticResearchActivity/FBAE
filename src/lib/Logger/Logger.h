#pragma once

#include <string>

class LoggerInstance;

class Logger {
public:
    explicit Logger();

    static LoggerInstance instance(std::string caller);
    void logTrace(const std::string && caller, const std::string && message) const;
    void logInfo(const std::string && caller, const std::string && message) const;
    void logError(const std::string && caller, const std::string && message) const;
    void logDebug(const std::string && caller, const std::string && message) const;
    void logWarn(const std::string && caller, const std::string && message) const;
    void logFatal(const std::string && caller, const std::string && message) const;
};

void initializeLogger();
const Logger &getLogger();

//                          |
//                          v annoying to call each time....
// getLogger().logInfo("myFunction", "We are running my function!");

class LoggerInstance {
public:
    explicit LoggerInstance(std::string callerName);

    void logTrace(const std::string && message) const;
    void logInfo(const std::string && message) const;
    void logError(const std::string && message) const;
    void logDebug(const std::string && message) const;
    void logWarn(const std::string && message) const;
    void logFatal(const std::string && message) const;
private:
    std::string callerName;
};

// auto logger = getLogger().instance("myFunction");
// logger.logInfo("We are running my function!");

