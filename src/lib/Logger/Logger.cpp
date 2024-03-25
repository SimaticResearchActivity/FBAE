#include "Logger.h"

#include <utility>

Logger::Logger() = default;

void Logger::logTrace(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_INFO(loggerInternal, message);
}

void Logger::logInfo(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_INFO(loggerInternal, message);
}

void Logger::logError(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_ERROR(loggerInternal, message);
}

void Logger::logDebug(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_DEBUG(loggerInternal, message);
}

void Logger::logWarn(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_WARN(loggerInternal, message);
}

void Logger::logFatal(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_FATAL(loggerInternal, message);
}

LoggerInstance Logger::instance(std::string caller) {
    return LoggerInstance(std::move(caller));
}


static Logger logger;

void initializeLogger() {
    logger = Logger();
}
const Logger &getLogger() {
    return logger;
}

LoggerInstance::LoggerInstance(std::string name) : instanceName(std::move(name)) {}

void LoggerInstance::logTrace(const std::string && message) const {
    getLogger().logTrace(
            static_cast<const std::string &&>(instanceName),
            static_cast<const std::string &&>(message));
}

void LoggerInstance::logInfo(const std::string && message) const {
    getLogger().logInfo(
            static_cast<const std::string &&>(instanceName),
            static_cast<const std::string &&>(message));
}

void LoggerInstance::logError(const std::string && message) const {
    getLogger().logError(
            static_cast<const std::string &&>(instanceName),
            static_cast<const std::string &&>(message));
}

void LoggerInstance::logDebug(const std::string && message) const {
    getLogger().logDebug(
            static_cast<const std::string &&>(instanceName),
            static_cast<const std::string &&>(message));
}

void LoggerInstance::logWarn(const std::string && message) const {
    getLogger().logWarn(
            static_cast<const std::string &&>(instanceName),
            static_cast<const std::string &&>(message));
}

void LoggerInstance::logFatal(const std::string && message) const {
    getLogger().logFatal(
            static_cast<const std::string &&>(instanceName),
            static_cast<const std::string &&>(message));
}
