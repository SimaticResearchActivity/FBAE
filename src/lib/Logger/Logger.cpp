#include "Logger.h"

Logger::Logger() = default;

void Logger::logTrace(const std::string && caller, const std::string && message) {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_INFO(loggerInternal, message);
}

void Logger::logInfo(const std::string && caller, const std::string && message) {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_INFO(loggerInternal, message);
}

void Logger::logError(const std::string && caller, const std::string && message) {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_ERROR(loggerInternal, message);
}

void Logger::logDebug(const std::string && caller, const std::string && message) {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_DEBUG(loggerInternal, message);
}

void Logger::logWarn(const std::string && caller, const std::string && message) {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_WARN(loggerInternal, message);
}

void Logger::logFatal(const std::string && caller, const std::string && message) {
    // auto loggerInternal = log4cxx::Logger::getLogger(caller);
    // LOG4CXX_FATAL(loggerInternal, message);
}

static Logger logger;

void initializeLogger() {
    logger = Logger();
}
const Logger &getLogger() {
    return logger;
}
