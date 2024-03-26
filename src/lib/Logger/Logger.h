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


    /** @brief Log a trace log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = Logger::instance("myFunction");
     * logger.trace("This is my trace message.");
     * */
    [[maybe_unused]] void trace(const std::string && message) const;

    /** @brief Log a info log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = Logger::instance("myFunction");
     * logger.info("This is my info message.");
     * */
    [[maybe_unused]] void info(const std::string && message) const;

    /** @brief Log a error log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = Logger::instance("myFunction");
     * logger.error("This is my error message.");
     * */
    [[maybe_unused]] void error(const std::string && message) const;

    /** @brief Log a debug log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = Logger::instance("myFunction");
     * logger.debug("This is my debug message.");
     * */
    [[maybe_unused]] void debug(const std::string && message) const;

    /** @brief Log a warning log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = Logger::instance("myFunction");
     * logger.warn("This is my warning message.");
     * */
    [[maybe_unused]] void warn(const std::string && message) const;

    /** @brief Log a fatal log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = Logger::instance("myFunction");
     * logger.fatal("This is my fatal message.");
     * */
    [[maybe_unused]] void fatal(const std::string && message) const;
private:
    /**
     * @brief the name of the function who initiated the log.
     */
    std::string callerName;
};

/**
 * @brief A logger used to display a variety of information to the
 * end user.
 */

namespace Logger {
    /**
     * @brief Instantiate a logger instance bound to a caller.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @return the instance of the logger bound to the caller.
     *
     * @example
     * auto logger = Logger::instance("myFunction");
     * logger.info("My function was called.")
     */
    LoggerInstance instance(std::string caller) noexcept;

    /**
     * @brief Instantiate a logger instance bound to a caller of a particular site.

     * @param rank[in]: the rank of the site that initiated the log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @return the instance of the logger bound to the caller and it's site.
     *
     * @example
     * auto logger = Logger::instanceOnSite(myRank, "myFunction");
     * logger.info("My function was called.");
     */
    [[maybe_unused]] LoggerInstance instanceOnSite(rank_t rank, const std::string &caller) noexcept;

    /** @brief Log a trace log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * Logger::trace("myFunction", "This is my trace message.");
     * */
    void trace(const std::string && caller, const std::string && message) noexcept;

    /** @brief Log a info log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * Logger::info("myFunction", "This is my info message.");
     * */
    void info(const std::string && caller, const std::string && message) noexcept;

    /** @brief Log a error log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * Logger::error("myFunction", "This is my error message.");
     * */
    void error(const std::string && caller, const std::string && message) noexcept;

    /** @brief Log a Debug log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * Logger::debug("myFunction", "This is my debug message.");
     * */
    void debug(const std::string && caller, const std::string && message) noexcept;

    /** @brief Log a warning log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * Logger::warn("myFunction", "This is my warning message.");
     * */
    void warn(const std::string && caller, const std::string && message) noexcept;

    /** @brief Log a fatal log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * Logger::fatal("myFunction", "This is my fatal message.");
     * */
    void fatal(const std::string && caller, const std::string && message) noexcept;
}


//class Logger {
//public:
//    explicit Logger();
//
//    /**
//     * @brief Instantiate a logger instance bound to a caller.
//     *
//     * @param caller[in]: the function that initiated the log.
//     *
//     * @return the instance of the logger bound to the caller.
//     *
//     * @example
//     * auto logger = Logger::instance("myFunction");
//     * logger.info("My function was called.")
//     */
//    [[maybe_unused]] static LoggerInstance instance(std::string caller);
//
//    /**
//     * @brief Instantiate a logger instance bound to a caller of a particular site.
//
//     * @param rank[in]: the rank of the site that initiated the log.
//     *
//     * @param caller[in]: the function that initiated the log.
//     *
//     * @return the instance of the logger bound to the caller and it's site.
//     *
//     * @example
//     * auto logger = Logger::instanceOnSite(myRank, "myFunction");
//     * logger.info("My function was called.");
//     */
//    [[maybe_unused]] static LoggerInstance instanceOnSite(rank_t rank, const std::string &caller);
//
//    /** @brief Log a trace log.
//     *
//     * @param caller[in]: the function that initiated the log.
//     *
//     * @param message[in]: the message we want to log.
//     *
//     * @example
//     * Logger::trace("myFunction", "This is my trace message.");
//     * */
//    void trace(const std::string && caller, const std::string && message) const;
//
//    /** @brief Log an info log.
//     *
//     * @param caller[in]: the function that initiated the log.
//     *
//     * @param message[in]: the message we want to log.
//     *
//     * @example
//     * Logger::info("myFunction", "This is my info message.");
//     * */
//    void info(const std::string && caller, const std::string && message) const;
//
//    /** @brief Log an error log.
//     *
//     * @param caller[in]: the function that initiated the log.
//     *
//     * @param message[in]: the message we want to log.
//     *
//     * @example
//     * Logger::error("myFunction", "This is my error message.");
//     * */
//    void error(const std::string && caller, const std::string && message) const;
//
//    /** @brief Log a debug log.
//     *
//     * @param caller[in]: the function that initiated the log.
//     *
//     * @param message[in]: the message we want to log.
//     *
//     * @example
//     * Logger::debug("myFunction", "This is my debug message.");
//     * */
//    void debug(const std::string && caller, const std::string && message) const;
//
//    /** @brief Log a warning log.
//     *
//     * @param caller[in]: the function that initiated the log.
//     *
//     * @param message[in]: the message we want to log.
//     *
//     * @example
//     * Logger::warn("myFunction", "This is my warning message.");
//     * */
//    void warn(const std::string && caller, const std::string && message) const;
//
//    /** @brief Log a fatal log.
//     *
//     * @param caller[in]: the function that initiated the log.
//     *
//     * @param message[in]: the message we want to log.
//     *
//     * @example
//     * Logger::fatal("myFunction", "This is my fatal message.");
//     * */
//    void fatal(const std::string && caller, const std::string && message) const;
//};
//
////void initializeLogger();
//
///**
// * @brief Getter for the internal logger singleton.
// *
// * @return A constant reference to the logger singleton.
// *
// * @example
// * Logger::info("myFunction", "My function has been called!");
// */
//const Logger &getLogger();

