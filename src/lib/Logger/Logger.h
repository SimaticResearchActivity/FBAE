#pragma once

#include "../basicTypes.h"

#include <string>

/// How to use the logger:
/// We either call the actual logging functions
/// by passing in the caller (usually just the function) as well as the
/// message:
///
/// Logger::info("myFunction", "We are running my function!");
///
/// Or we can bind the caller to an intermediate object called a LoggerInstance,
/// and call these functions onto this state directly to avoid typing the caller
/// multiple times:
///
/// const auto logger = Logger::instance("myFunction");
/// logger.info("We are running my function!");
/// logger.info("Here too!");
///
/// When dealing with sites, we could also bind the site's rank to the caller
/// using the following function:
///
/// const auto logger = Logger::instanceOnSite(myRank, "MyFunction");
/// logger.info("We are running my function on my rank!");
///
/// The list of all the logging types are: trace, info, error, debug, warn and fatal.

class LoggerInstance {
public:

    /**
     * @brief Constructor for a LoggerInstance.
     *
     * @param callerName[in]: the name of the function (or part
     * of code) that initiated the log.
     */
    explicit LoggerInstance(std::string callerName) noexcept;

    /**
     * @brief Log a trace from the function or part of code this
     * LoggerInstance was bound to.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * const auto logger = Logger::instance("myFunction");
     * logger.trace("This is a trace from myFunction!");
     */
    [[maybe_unused]] void trace(const std::string && message) const noexcept;

    /**
     * @brief Log information from the function or part of code this
     * LoggerInstance was bound to.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * const auto logger = Logger::instance("myFunction");
     * logger.info("This is information from myFunction!");
     */
    [[maybe_unused]] void info(const std::string && message) const noexcept;

    /**
     * @brief Log an error from the function or part of code this
     * LoggerInstance was bound to.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * const auto logger = Logger::instance("myFunction");
     * logger.error("This is an error from myFunction!");
     */
    [[maybe_unused]] void error(const std::string && message) const noexcept;

    /**
     * @brief Log debug information from the function or part of code this
     * LoggerInstance was bound to.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * const auto logger = Logger::instance("myFunction");
     * logger.debug("This is debug information from myFunction!");
     */
    [[maybe_unused]] void debug(const std::string && message) const noexcept;

    /**
     * @brief Log a warning from the function or part of code this
     * LoggerInstance was bound to.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * const auto logger = Logger::instance("myFunction");
     * logger.warning("This is a warning from myFunction!");
     */
    [[maybe_unused]] void warn(const std::string && message) const noexcept;

    /**
     * @brief Log an fatal error from the function or part of code this
     * LoggerInstance was bound to.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * const auto logger = Logger::instance("myFunction");
     * logger.fatal("This is an fatal error from myFunction!");
     */
    [[maybe_unused]] void fatal(const std::string && message) const noexcept;
private:

    /**
     * The name of the caller (usually the function) we want
     * to log information from.
     */
    std::string callerName;
};

namespace Logger {

    /** @brief Sets up the logger.
     *
     * This function sets up the underlying library log4cxx. It is
     * required to call this function once before any call to functions
     * related to logging.
     */
    void setupLogger();

    /**
     * @brief Instance a LoggerInstance bound by a caller name.
     *
     * @param caller[in]: the name of the caller from whom we want
     * to log information.
     *
     * @return: The instance of LoggerInstance we want to use.
     *
     * @example:
     * const auto logger = Logger::instance("myFunction");
     */
    LoggerInstance instance(std::string caller) noexcept;

    /**
     * @brief Instance a LoggerInstance bound by a caller rank and name.
     *
     * This function should only be used when the logging is done on a
     * specific site.
     *
     * @param rank[in]: the rank of the site from whom we want to log
     * information.
     *
     * @param caller[in]: the name of the caller from whom we want
     * to log information.
     *
     * @return: The instance of LoggerInstance we want to use.
     *
     * @example
     * // If we are in a SessionLayer
     * const auto logger = Logger::instance(getRank(), "myFunction");
     *
     * // If we are in an AlgoLayer
     * const auto logger = Logger::instance(getSessionLayer().getRank(), "myFunction");
     */
    [[maybe_unused]] LoggerInstance instanceOnSite(rank_t rank, const std::string &caller) noexcept;

    /**
     * @brief Log a trace from the function or part of code that
     * was provided to the caller parameter.
     *
     * @param caller[in]: The name of the caller or function
     * who initiated the log.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * Logger::trace("myFunction", "This is a trace from myFunction!");
     */
    void trace(const std::string && caller, const std::string && message) noexcept;

    /**
     * @brief Log information from the function or part of code that
     * was provided to the caller parameter.
     *
     * @param caller[in]: The name of the caller or function
     * who initiated the log.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * Logger::info("myFunction", "This is information from myFunction!");
     */
    void info(const std::string && caller, const std::string && message) noexcept;

    /**
     * @brief Log an error from the function or part of code that
     * was provided to the caller parameter.
     *
     * @param caller[in]: The name of the caller or function
     * who initiated the log.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * Logger::error("myFunction", "This is an error from myFunction!");
     */
    void error(const std::string && caller, const std::string && message) noexcept;

    /**
     * @brief Log debug information from the function or part of code that
     * was provided to the caller parameter.
     *
     * @param caller[in]: The name of the caller or function
     * who initiated the log.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * Logger::debug("myFunction", "This is debug information from myFunction!");
     */
    void debug(const std::string && caller, const std::string && message) noexcept;

    /**
     * @brief Log a warning from the function or part of code that
     * was provided to the caller parameter.
     *
     * @param caller[in]: The name of the caller or function
     * who initiated the log.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * Logger::warning("myFunction", "This is a warning from myFunction!");
     */
    void warn(const std::string && caller, const std::string && message) noexcept;

    /**
     * @brief Log a fatal error from the function or part of code that
     * was provided to the caller parameter.
     *
     * @param caller[in]: The name of the caller or function
     * who initiated the log.
     *
     * @param message[in]: The message we want to log.
     *
     * @example
     * Logger::fatal("myFunction", "This is a fatal error from myFunction!");
     */
    void fatal(const std::string && caller, const std::string && message) noexcept;
}