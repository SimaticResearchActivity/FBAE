#pragma once

// Guard in case people use this macro outside of this header file.
#ifdef LOGGER_TYPE_STREAM
    #undef LOGGER_TYPE_STREAM
#endif // LOGGER_TYPE_STREAM

// Define this to get the stream logger.
#define LOGGER_TYPE_STREAM