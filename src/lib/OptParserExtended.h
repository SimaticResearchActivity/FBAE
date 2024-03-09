// Additional functions to options.h
#pragma once

#include "options.h"

namespace mlib {
    class OptParserExtended: public OptParser
    {
    public:
        OptParserExtended (std::initializer_list<const char*> list);

        [[nodiscard]] std::string getoptStringRequired(char option) const;
        [[nodiscard]] int getoptIntRequired(char option) const;
    };
}
