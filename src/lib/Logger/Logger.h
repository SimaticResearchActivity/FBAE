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

class LoggerInstance {
public:
    explicit LoggerInstance(std::string callerName);

    [[maybe_unused]] void logTrace(const std::string && message) const;
    [[maybe_unused]] void logInfo(const std::string && message) const;
    [[maybe_unused]] void logError(const std::string && message) const;
    [[maybe_unused]] void logDebug(const std::string && message) const;
    [[maybe_unused]] void logWarn(const std::string && message) const;
    [[maybe_unused]] void logFatal(const std::string && message) const;
private:
    std::string callerName;
};

//                          |
//                          v annoying to call each time....
// getLogger().logInfo("myFunction", "We are running my function!");
//
// Or we can do:
//
// auto logger = getLogger().instance("myFunction");
// logger.logInfo("We are running my function!");

