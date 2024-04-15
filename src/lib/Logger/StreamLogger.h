#pragma once

#include <string>
#include <sstream>


namespace Logger {
    enum StreamType {
        Trace, Info, Warn, Error, Fatal, Debug
    };

    [[maybe_unused]] static const std::string trace = "T\0";
    [[maybe_unused]] static const std::string info = "I\0";
    [[maybe_unused]] static const std::string warn = "W\0";
    [[maybe_unused]] static const std::string error = "E\0";
    [[maybe_unused]] static const std::string fatal = "F\0";
    [[maybe_unused]] static const std::string debug = "D\0";
    [[maybe_unused]] static const std::string endLog = "\0";
}

struct StreamInstance {
public:
    StreamInstance(std::string  callerName, Logger::StreamType streamType);

    // Pushes to a stream
    template<typename T> inline StreamInstance &operator<<(const T &value) {
        std::stringstream buffer;
        buffer << value;
        std::string stringRepr = buffer.str();
        stringRepr == "\0" ? terminate() : (void)(messageBuffer << stringRepr);
        return *this;
    }


    void terminate();
private:

    std::stringstream messageBuffer;
    std::string callerName;
    Logger::StreamType streamType;
};

namespace Logger {
    StreamInstance instanceStream(StreamType streamType, const std::string & callerName);
    void setupLogger();
}

