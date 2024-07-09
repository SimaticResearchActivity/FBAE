// Additional functions to options.h
#pragma once

#include "./Logger/LoggerConfig.h"
#include "options.h"

namespace mlib {
class OptParserExtended : public OptParser {
 public:
  OptParserExtended(std::initializer_list<const char*> list);

  [[nodiscard]] std::string getoptStringRequired(
      char option, fbae::LoggerPtr const& logger) const;
  [[nodiscard]] int getoptIntRequired(char option,
                                      fbae::LoggerPtr const& logger) const;
};
}  // namespace mlib
