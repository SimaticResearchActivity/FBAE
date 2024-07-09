// Additional functions to options.h
#pragma once

#include "./Logger/LoggerConfig.h"
#include "options.h"

namespace fbae::core {

class OptParserExtended : public OptParser {
 public:
  OptParserExtended(std::initializer_list<const char*> list);

  [[nodiscard]] std::string getoptStringRequired(
      char option, fbae::core::Logger::LoggerPtr const& logger) const;
  [[nodiscard]] int getoptIntRequired(char option,
                                      fbae::core::Logger::LoggerPtr const& logger) const;
};

}  // namespace fbae::core
