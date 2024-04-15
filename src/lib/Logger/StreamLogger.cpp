#include "StreamLogger.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <iostream>
#include <utility>

void Logger::setupLogger() {
    log4cxx::BasicConfigurator::configure();
}

StreamInstance::StreamInstance(std::string callerName, Logger::StreamType streamType) :
    callerName(std::move(callerName)), streamType(streamType) {}

[[maybe_unused]] StreamInstance Logger::instanceStream(StreamType streamType, const std::string & callerName) {
    return StreamInstance(
            callerName,
            streamType
    );
}

void StreamInstance::terminate() {
    const std::string completeMessage = messageBuffer.str();
    auto loggerInternal = log4cxx::Logger::getLogger(callerName);
    switch (streamType) {
        case Logger::StreamType::Trace:
            LOG4CXX_TRACE(loggerInternal, completeMessage);
            break;
        case Logger::StreamType::Info:
            LOG4CXX_INFO(loggerInternal, completeMessage);
            break;
        case Logger::StreamType::Warn:
            LOG4CXX_WARN(loggerInternal, completeMessage);
            break;
        case Logger::StreamType::Error:
            LOG4CXX_ERROR(loggerInternal, completeMessage);
            break;
        case Logger::StreamType::Fatal:
            LOG4CXX_FATAL(loggerInternal, completeMessage);
            break;
        case Logger::StreamType::Debug:
            LOG4CXX_DEBUG(loggerInternal, completeMessage);
            break;
        default:
            loggerInternal = log4cxx::Logger::getLogger("Logger::StreamInstance::operator<<(StreamTerminator)");
            std::stringstream buffer;
            buffer << "Invalid stream type: " << streamType << ". Message to display: \"" << completeMessage << "\"";
            LOG4CXX_ERROR(loggerInternal, buffer.str());
            break;
    }

    messageBuffer.str(std::string());
}
