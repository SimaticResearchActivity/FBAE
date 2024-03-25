#pragma once

#include "../basicTypes.h"

#include <string>

// How to use the logger:
// We either call the getter and call one of the available log methods
// by passing in the caller (usually just the function) as well as the
// message:
//
// getLogger().logInfo("myFunction", "We are running my function!");
//
//
// Or we can bind the caller to a intermediate object called a LoggerInstance,
// and call these functions onto this state directly to avoid typing the caller
// multiple times:
//
// auto logger = getLogger().instance("myFunction");
// logger.logInfo("We are running my function!");
// logger.logInfo("Here too!");
//
// When dealing with sites, we could also bind the site's rank to the caller
// using the following function:
//
// auto logger = getLogger().instanceOnSite(myRank, "MyFunction");
// logger.logInfo("We are running my function on my rank!");

class LoggerInstance;

/**
 * @brief A logger used to display a variety of information to the
 * end user.
 */
class Logger {
public:
    explicit Logger();

    /**
     * @brief Instantiate a logger instance bound to a caller.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @return the instance of the logger bound to the caller.
     *
     * @example
     * auto logger = getLogger().instance("myFunction");
     * logger.logInfo("My function was called.")
     */
    [[maybe_unused]] static LoggerInstance instance(std::string caller);

    /**
     * @brief Instantiate a logger instance bound to a caller of a particular site.

     * @param rank[in]: the rank of the site that initiated the log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @return the instance of the logger bound to the caller and it's site.
     *
     * @example
     * auto logger = getLogger().instanceOnSite(myRank, "myFunction");
     * logger.logInfo("My function was called.");
     */
    [[maybe_unused]] static LoggerInstance instanceOnSite(rank_t rank, const std::string &caller);

    /** @brief Log a trace log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * getLogger().logTrace("myFunction", "This is my trace message.");
     * */
    void logTrace(const std::string && caller, const std::string && message) const;

    /** @brief Log a info log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * getLogger().logInfo("myFunction", "This is my info message.");
     * */
    void logInfo(const std::string && caller, const std::string && message) const;

    /** @brief Log a error log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * getLogger().logError("myFunction", "This is my error message.");
     * */
    void logError(const std::string && caller, const std::string && message) const;

    /** @brief Log a debug log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * getLogger().logDebug("myFunction", "This is my debug message.");
     * */
    void logDebug(const std::string && caller, const std::string && message) const;

    /** @brief Log a warning log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * getLogger().logWarn("myFunction", "This is my warning message.");
     * */
    void logWarn(const std::string && caller, const std::string && message) const;

    /** @brief Log a fatal log.
     *
     * @param caller[in]: the function that initiated the log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * getLogger().logFatal("myFunction", "This is my fatal message.");
     * */
    void logFatal(const std::string && caller, const std::string && message) const;
};

//void initializeLogger();

/**
 * @brief Getter for the internal logger singleton.
 *
 * @return A constant reference to the logger singleton.
 *
 * @example
 * getLogger().logInfo("myFunction", "My function has been called!");
 */
const Logger &getLogger();

class LoggerInstance {
public:

    explicit LoggerInstance(std::string callerName);


    /** @brief Log a trace log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = getLogger().instance("myFunction");
     * logger.logTrace("This is my trace message.");
     * */
    [[maybe_unused]] void logTrace(const std::string && message) const;

    /** @brief Log a info log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = getLogger().instance("myFunction");
     * logger.logInfo("This is my info message.");
     * */
    [[maybe_unused]] void logInfo(const std::string && message) const;

    /** @brief Log a error log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = getLogger().instance("myFunction");
     * logger.logError("This is my error message.");
     * */
    [[maybe_unused]] void logError(const std::string && message) const;

    /** @brief Log a debug log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = getLogger().instance("myFunction");
     * logger.logDebug("This is my debug message.");
     * */
    [[maybe_unused]] void logDebug(const std::string && message) const;

    /** @brief Log a warning log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = getLogger().instance("myFunction");
     * logger.logWarning("This is my warning message.");
     * */
    [[maybe_unused]] void logWarn(const std::string && message) const;

    /** @brief Log a fatal log.
     *
     * @param message[in]: the message we want to log.
     *
     * @example
     * auto logger = getLogger().instance("myFunction");
     * logger.logFatal("This is my fatal message.");
     * */
    [[maybe_unused]] void logFatal(const std::string && message) const;
private:
    /**
     * @brief the name of the function who initiated the log.
     */
    std::string callerName;
};

