#pragma once

#include "../basicTypes.h"

#include <string>

// How to use the logger:
// We either call the actual logging functions
// by passing in the caller (usually just the function) as well as the
// message:
//
// Logger::info("myFunction", "We are running my function!");
//
// Or we can bind the caller to an intermediate object called a LoggerInstance,
// and call these functions onto this state directly to avoid typing the caller
// multiple times:
//
// auto logger = Logger::instance("myFunction");
// logger.info("We are running my function!");
// logger.info("Here too!");
//
// When dealing with sites, we could also bind the site's rank to the caller
// using the following function:
//
// auto logger = Logger::instanceOnSite(myRank, "MyFunction");
// logger.info("We are running my function on my rank!");
//
// The list of all the logging types are: trace, info, error, debug, warn and fatal.

class LoggerInstance {
public:

    explicit LoggerInstance(std::string callerName);

    [[maybe_unused]] void trace(const std::string && message) const;
    [[maybe_unused]] void info(const std::string && message) const;
    [[maybe_unused]] void error(const std::string && message) const;
    [[maybe_unused]] void debug(const std::string && message) const;
    [[maybe_unused]] void warn(const std::string && message) const;
    [[maybe_unused]] void fatal(const std::string && message) const;
private:

    std::string callerName;
};

namespace Logger {
    LoggerInstance instance(std::string caller) noexcept;
    [[maybe_unused]] LoggerInstance instanceOnSite(rank_t rank, const std::string &caller) noexcept;


    void trace(const std::string && caller, const std::string && message) noexcept;
    void info(const std::string && caller, const std::string && message) noexcept;
    void error(const std::string && caller, const std::string && message) noexcept;
    void debug(const std::string && caller, const std::string && message) noexcept;
    void fatal(const std::string && caller, const std::string && message) noexcept;
    void warn(const std::string && caller, const std::string && message) noexcept;
}