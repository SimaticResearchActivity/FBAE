#pragma once

// LOGGER_TYPE_BASIC: simple logging, functions take
// a simple string as an input.

// LOGGER_TYPE_STREAM: more advanced, akin to std::cout.

// Make sure no-one has defined LOGGER_TYPE_STREAM
// or LOGGER_TYPE_BASIC beforehand, as this may
// cause strange behavior, especially if this header
// file is included in multiple other files.
#ifdef LOGGER_TYPE_STREAM
    #undef LOGGER_TYPE_STREAM
#endif // LOGGER_TYPE_STREAM

#ifdef LOGGER_TYPE_BASIC
    #undef LOGGER_TYPE_BASIC
#endif // LOGGER_TYPE_BASIC

// Choose one or the other. If both are chosen
// LOGGER_TYPE_STREAM will end up winning.
#define LOGGER_TYPE_STREAM
// #define LOGGER_TYPE_BASIC

#ifdef LOGGER_TYPE_STREAM
    #include "StreamLogger.h"
#else // LOGGER_TYPE_STREAM
    #include "BasicLogger.h"
#endif // LOGGER_TYPE_STREAM
