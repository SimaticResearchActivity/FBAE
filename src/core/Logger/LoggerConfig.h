#pragma once
#include <log4cxx/logger.h>

namespace fbae::core::Logger {

using LoggerPtr = log4cxx::LoggerPtr;

extern auto getLogger(const std::string& name = std::string()) -> LoggerPtr;

}  // namespace fbae::core::Logger
