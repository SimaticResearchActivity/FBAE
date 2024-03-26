#include "Logger.h"

#include <utility>
#include <sstream>

inline void Logger::trace(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_TRACE(loggerInternal, message);
}

inline void Logger::info(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_INFO(loggerInternal, message);
}

inline void Logger::error(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_ERROR(loggerInternal, message);
}

inline void Logger::debug(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_DEBUG(loggerInternal, message);
}

inline void Logger::warn(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_WARN(loggerInternal, message);
}

inline void Logger::fatal(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_FATAL(loggerInternal, message);
}

[[maybe_unused]] inline LoggerInstance Logger::instance(std::string caller) noexcept {
    return LoggerInstance(std::move(caller));
}

[[maybe_unused]] inline LoggerInstance Logger::instanceOnSite(rank_t rank, const std::string &caller) noexcept {
    std::stringstream stream;
    stream << "Rank " << std::to_string(static_cast<uint32_t>(rank)) <<  ": " << caller;
    return LoggerInstance(stream.str());
}


//static Logger logger = Logger();

//inline void initializeLogger() {
//    logger = Logger();
//}
//inline const Logger &getLogger() {
//    return logger;
//}

inline LoggerInstance::LoggerInstance(std::string callerName) : callerName(std::move(callerName)) {}


[[maybe_unused]] inline void LoggerInstance::trace(const std::string && message) const {
    Logger::trace(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::info(const std::string && message) const {
    Logger::info(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::error(const std::string && message) const {
    Logger::error(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::debug(const std::string && message) const {
    Logger::debug(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::warn(const std::string && message) const {
    Logger::warn(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] inline void LoggerInstance::fatal(const std::string && message) const {
    Logger::fatal(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}
