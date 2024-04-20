#pragma once

#include "LoggerConfig.h"

#ifdef LOGGER_TYPE_STREAM
    #include "StreamLogger.h"
#else // LOGGER_TYPE_STREAM
    #include "BasicLogger.h"
#endif // LOGGER_TYPE_STREAM
