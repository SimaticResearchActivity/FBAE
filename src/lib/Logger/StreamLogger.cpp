#include "Logger.h"

#ifdef LOGGER_TYPE_STREAM

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <iostream>
#include <utility>

void Logger::setupLogger() {
    log4cxx::BasicConfigurator::configure();
}

Logger::StreamInstance::StreamInstance(std::string callerName, StreamType streamType) :
    callerName(std::move(callerName)), streamType(streamType) {}

[[maybe_unused]] Logger::StreamInstance Logger::instanceStream(StreamType streamType, const std::string & callerName) {
    return StreamInstance(callerName, streamType);
}

void Logger::StreamInstance::terminate() {
    const std::string completeMessage = messageBuffer.str();
    auto loggerInternal = log4cxx::Logger::getLogger(callerName);
    switch (streamType) {
        case StreamType::Trace:
            LOG4CXX_TRACE(loggerInternal, completeMessage);
            break;
        case StreamType::Info:
            LOG4CXX_INFO(loggerInternal, completeMessage);
            break;
        case StreamType::Warn:
            LOG4CXX_WARN(loggerInternal, completeMessage);
            break;
        case StreamType::Error:
            LOG4CXX_ERROR(loggerInternal, completeMessage);
            break;
        case StreamType::Fatal:
            LOG4CXX_FATAL(loggerInternal, completeMessage);
            break;
        case StreamType::Debug:
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

Logger::StreamInstanceBuilder::StreamInstanceBuilder(std::string callerName) noexcept : callerName(std::move(callerName)) {}

Logger::StreamInstance Logger::StreamInstanceBuilder::trace() {
    return StreamInstance(callerName, StreamType::Trace);
}

Logger::StreamInstance Logger::StreamInstanceBuilder::info() {
    return StreamInstance(callerName, StreamType::Info);
}

Logger::StreamInstance Logger::StreamInstanceBuilder::warn() {
    return StreamInstance(callerName, StreamType::Warn);
}

Logger::StreamInstance Logger::StreamInstanceBuilder::error() {
    return StreamInstance(callerName, StreamType::Error);
}

Logger::StreamInstance Logger::StreamInstanceBuilder::fatal() {
    return StreamInstance(callerName, StreamType::Fatal);
}

Logger::StreamInstance Logger::StreamInstanceBuilder::debug() {
    return StreamInstance(callerName, StreamType::Debug);
}

Logger::StreamInstanceBuilder Logger::instance(std::string callerName) noexcept {
    return StreamInstanceBuilder(std::move(callerName));
}

Logger::StreamInstanceBuilder Logger::instanceOnSite(const std::string &callerName, rank_t rank) noexcept {
    std::stringstream buffer;
    buffer << "Rank " << std::to_string(static_cast<uint32_t>(rank)) <<  ": " << callerName;
    return StreamInstanceBuilder(buffer.str());
}


#endif // LOGGER_TYPE_STREAM