#include "Logger.h"

#include <utility>
#include <sstream>

Logger::Logger() = default;

inline void Logger::logTrace(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_TRACE(loggerInternal, message);
}

inline void Logger::logInfo(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_INFO(loggerInternal, message);
}

inline void Logger::logError(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_ERROR(loggerInternal, message);
}

inline void Logger::logDebug(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_DEBUG(loggerInternal, message);
}

inline void Logger::logWarn(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_WARN(loggerInternal, message);
}

inline void Logger::logFatal(const std::string && caller, const std::string && message) const {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_FATAL(loggerInternal, message);
}

[[maybe_unused]] inline LoggerInstance Logger::instance(std::string caller) {
    return LoggerInstance(std::move(caller));
}

[[maybe_unused]] inline LoggerInstance Logger::instanceOnSite(rank_t rank, const std::string &caller) {
    std::stringstream stream;
    stream << "Rank " << std::to_string(static_cast<uint32_t>(rank)) <<  ": " << caller;
    return LoggerInstance(stream.str());
}


static Logger logger = Logger();

//inline void initializeLogger() {
//    logger = Logger();
//}
inline const Logger &getLogger() {
    return logger;
}

inline LoggerInstance::LoggerInstance(std::string callerName) : callerName(std::move(callerName)) {}


[[maybe_unused]] inline void LoggerInstance::logTrace(const std::string && message) const {
    getLogger().logTrace(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::logInfo(const std::string && message) const {
    getLogger().logInfo(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::logError(const std::string && message) const {
    getLogger().logError(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::logDebug(const std::string && message) const {
    getLogger().logDebug(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::logWarn(const std::string && message) const {
    getLogger().logWarn(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::logFatal(const std::string && message) const {
    getLogger().logFatal(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}
