#pragma once

#include "LoggerConfig.h"
#ifdef LOGGER_TYPE_STREAM

#include "basicTypes.h"

#include <string>
#include <sstream>

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

        [[maybe_unused]] StreamInstance trace();
        [[maybe_unused]] StreamInstance info();
        [[maybe_unused]] StreamInstance warn();
        [[maybe_unused]] StreamInstance error();
        [[maybe_unused]] StreamInstance fatal();
        [[maybe_unused]] StreamInstance debug();
    private:
        std::string callerName;
    };

    [[maybe_unused]] StreamInstanceBuilder instance(std::string callerName) noexcept;
    [[maybe_unused]] StreamInstanceBuilder instanceOnSite(const std::string &callerName, rank_t rank) noexcept;
    /**
     * @brief The function we call to create a instance of a StreamInstance.
     *
     * @param streamType [in] The type of the stream we wish to construct. See Logger::StreamType for more information.
     * @param callerName [in] The name of the function or piece of code who wishes to log information.
     * @return A instance of a StreamInstance corresponding to the parameters provided to the function.
     */
    StreamInstance instanceStream(StreamType streamType, const std::string &callerName);

    /**
     * @brief This function must be called before creating and using any StreamInstance.
     */
    void setupLogger();
}

#endif // LOGGER_TYPE_STREAM