#include "Logger.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

#include <utility>
#include <sstream>
#include <iostream>


void Logger::trace(const std::string && caller, const std::string && message) noexcept {
    auto loggerInternal = LOG4CXX_NS::Logger();
//     auto loggerInternal = log4cxx::Logger::getLogger(caller);
//     LOG4CXX_TRACE(loggerInternal, message);
    std::cout << "Trace from " << caller << ": " << message << std::endl;
}

void Logger::info(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_INFO(loggerInternal, message);
    std::cout << "Info from " << caller << ": " << message << std::endl;
}

void Logger::error(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_ERROR(loggerInternal, message);
    std::cout << "Error from " << caller << ": " << message << std::endl;
}

void Logger::debug(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_DEBUG(loggerInternal, message);
    std::cout << "Debug from " << caller << ": " << message << std::endl;
}

void Logger::warn(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_WARN(loggerInternal, message);
    std::cout << "Warning from " << caller << ": " << message << std::endl;
}

void Logger::fatal(const std::string && caller, const std::string && message) noexcept {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_FATAL(loggerInternal, message);
    std::cout << "Fatal error from " << caller << ": " << message << std::endl;
}

[[maybe_unused]] LoggerInstance Logger::instance(std::string caller) noexcept {
    return LoggerInstance(std::move(caller));
}

[[maybe_unused]] LoggerInstance Logger::instanceOnSite(rank_t rank, const std::string &caller) noexcept {
    std::stringstream stream;
    stream << "Rank " << std::to_string(static_cast<uint32_t>(rank)) <<  ": " << caller;
    return LoggerInstance(stream.str());
}

LoggerInstance::LoggerInstance(std::string callerName) noexcept : callerName(std::move(callerName)) {}


[[maybe_unused]] void LoggerInstance::trace(const std::string && message) const noexcept {
    Logger::trace(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] void LoggerInstance::info(const std::string && message) const noexcept {
    Logger::info(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] void LoggerInstance::error(const std::string && message) const noexcept {
    Logger::error(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] void LoggerInstance::debug(const std::string && message) const noexcept {
    Logger::debug(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] void LoggerInstance::warn(const std::string && message) const noexcept {
    Logger::warn(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}

[[maybe_unused]] void LoggerInstance::fatal(const std::string && message) const noexcept {
    Logger::fatal(
            static_cast<const std::string &&>(callerName),
            static_cast<const std::string &&>(message));
}
