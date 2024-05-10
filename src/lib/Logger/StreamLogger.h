#pragma once

#include "LoggerConfig.h"
#ifdef LOGGER_TYPE_STREAM

#include "basicTypes.h"

#include <string>
#include <sstream>


/*
 * How to use the stream logger:
 *
 * // create a logger by passing the name of the function
 * // we are currently in.
 * auto logger = Logger::instance("myFunction");
 *
 * // Or we can create a logger by passing in the name of the
 * // function we are currently in and the rank of the site we
 * // are currently in if we are in a SessionLayer, AlgoLayer
 * // or a CommLayer.
 * auto logger = Logger::instanceOnSite("myFunction", myRank);
 *
 * // We can log information like this:
 * logger.info() << "This is information." << Logger::endLog;
 *
 * Or using any of the following functions: trace(), info(), warn()
 * error(), fatal() or debug().
 *
 * The message will only be flushed once we pass Logger::endLog
 * to the logger. We shouldn't push anything after this item.
 *
 * Strings, numbers, and anything that can be displayed onto the screen
 * can be passed in these functions.
 */

namespace Logger {
    /**
     * @brief The type of log stream we want
     */
    enum StreamType {
        Trace, Info, Warn, Error, Fatal, Debug
    };

    /**
     * @brief The marker denoting the end of a log.
     *
     * Once this has been pushed onto the StreamInstance,
     * the log will be sent and the internal buffer will be
     * reset.
     */
     // This string should never appear under normal circumstances?
    [[maybe_unused]] static const std::string endLog = { '\xFF', '\0' };

    /**
     * @brief A instance of a Logging stream.
     *
     * Once this structure has been created, we will be able to push messages onto it,
     * and it will temporarily store it's contents until the log is finished (done by
     * pushing a Logger::endLog)
     */
    struct StreamInstance {
    public:
        explicit StreamInstance(std::string callerName, Logger::StreamType streamType);

        /**
         * @brief Push content onto the stream instance.
         *
         * @tparam T The type of the content we wish to push.
         *
         * @param value [in] The content we wish to push.
         *
         * @return A reference to the StreamInstance, useful for chaining
         * calls to this operator.
         */
        template<typename T> inline StreamInstance &operator<<(const T &value) {
            std::stringstream buffer;
            buffer << value;
            std::string stringRepr = buffer.str();
            stringRepr == Logger::endLog ? terminate() : (void)(messageBuffer << stringRepr);
            return *this;
        }

    private:
        void terminate();

        std::stringstream messageBuffer;
        std::string callerName;
        Logger::StreamType streamType;
    };

    struct StreamInstanceBuilder {
    public:
        explicit StreamInstanceBuilder(std::string callerName) noexcept;

        /**
         * @brief Initiate a trace log stream instance. We can push onto a log
         * stream however we wish, and the log will terminate when we
         * use Logger::endLog.
         *
         * @example:
         * auto logger = Logger::instance("myFunction");
         *
         * logger.trace() \<\< "My message" \<\< Logger::endLog;
         *
         * @return the instance of the trace stream.
         */
        [[maybe_unused]] StreamInstance trace();

        /**
         * @brief Initiate a info log stream instance. We can push onto a log
         * stream however we wish, and the log will terminate when we
         * use Logger::endLog.
         *
         * @example:
         * auto logger = Logger::instance("myFunction");
         *
         * logger.info() \<\< "My message" \<\< Logger::endLog;
         * @return the instance of the info stream.
         */
        [[maybe_unused]] StreamInstance info();

        /**
         * @brief Initiate a warn log stream instance. We can push onto a log
         * stream however we wish, and the log will terminate when we
         * use Logger::endLog.
         *
         * @example:
         * auto logger = Logger::instance("myFunction");
         *
         * logger.warn() \<\< "My message" \<\< Logger::endLog;
         *
         * @return the instance of the warn stream.
         */
        [[maybe_unused]] StreamInstance warn();

        /**
         * @brief Initiate an error log stream instance. We can push onto a log
         * stream however we wish, and the log will terminate when we
         * use Logger::endLog.
         *
         * @example:
         * auto logger = Logger::instance("myFunction");
         *
         * logger.error() \<\< "My message" \<\< Logger::endLog;
         *
         * @return the instance of the error stream.
         */
        [[maybe_unused]] StreamInstance error();

        /**
         * @brief Initiate a fatal log stream instance. We can push onto a log
         * stream however we wish, and the log will terminate when we
         * use Logger::endLog.
         *
         * @example:
         * auto logger = Logger::instance("myFunction");
         *
         * logger.fatal() \<\< "My message" \<\< Logger::endLog;
         *
         * @return the instance of the fatal stream.
         */
        [[maybe_unused]] StreamInstance fatal();

        /**
         * @brief Initiate a debug log stream instance. We can push onto a log
         * stream however we wish, and the log will terminate when we
         * use Logger::endLog.
         *
         * @example:
         * auto logger = Logger::instance("myFunction");
         *
         * logger.debug() \<\< "My message" \<\< Logger::endLog;
         *
         * @return the instance of the debug stream.
         */
        [[maybe_unused]] StreamInstance debug();

    private:
        std::string callerName;
    };

    /**
     * @brief Instance a new logger instance based off of a caller's name.
     *
     * @param callerName [in]: the name of the function that called this function.
     *
     * @return A new logger instance (of type StreamInstanceBuilder)
     *
     * @example:
     *
     * auto logger = Logger::instance("myFunction");
     *
     * logger.info() \<\< "This is some info" \<\< Logger::endLog;
     * logger.warn() \<\< "This is a warning" \<\< Logger::endLog;
     * // ...
     */
    [[maybe_unused]] StreamInstanceBuilder instance(std::string callerName) noexcept;

    /**
     * @brief Instance a new logger instance based off of a caller's name and the site's rank.
     *
     * @param callerName [in]: the name of the function that called this function.
     *
     * @param rank [in]: the rank of the site that called this function.
     *
     * @return A new logger instance (of type StreamInstanceBuilder)
     *
     * @example:
     *
     * auto logger = Logger::instanceOnSite("myFunction", 3);
     *
     * logger.info() \<\< "This is some info from rank 3" \<\< Logger::endLog;
     * logger.warn() \<\< "This is a warning from rank 3" \<\< Logger::endLog;
     * // ...
     */
    [[maybe_unused]] StreamInstanceBuilder instanceOnSite(const std::string &callerName, rank_t rank) noexcept;
    /**
     * @brief The function we call to create a instance of a StreamInstance.
     *
     *
     * This shouldn't be used unless we are sure we will only be logging a single thing in
     * this function. If not, we would prefer to use instance or instanceOnSite instead.
     *
     * @param streamType [in] The type of the stream we wish to construct. See Logger::StreamType for more information.
     * @param callerName [in] The name of the function or piece of code who wishes to log information.
     * @return A instance of a StreamInstance corresponding to the parameters provided to the function.
     *
     * @example:
     *
     * Logger::instanceStream(Logger::StreamType::Info, "myFunction") << "Here's some info" << Logger::endLog;
     */
    StreamInstance instanceStream(StreamType streamType, const std::string &callerName);

    /**
     * @brief This function must be called before creating and using any StreamInstance.
     */
    void setupLogger();
}

#endif // LOGGER_TYPE_STREAM