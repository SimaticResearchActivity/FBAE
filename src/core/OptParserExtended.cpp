#include "OptParserExtended.h"

#include <format>
#include <sstream>

namespace fbae::core {

OptParserExtended::OptParserExtended(std::initializer_list<const char*> list)
    : OptParser{list} {}

std::string OptParserExtended::getoptStringRequired(
    char option, Logger::LoggerPtr const& logger) const {
  std::string optArg;
  if (!getopt(option, optArg)) {
    LOG4CXX_FATAL_FMT(logger, "Option -{} is missing\n\nUsage:\n{}\nWhere:\n{}",
                      option, synopsis(), description());
    exit(1);
  }
  return optArg;
}

int OptParserExtended::getoptIntRequired(char option,
                                         Logger::LoggerPtr const& logger) const {
  std::string optArg;
  if (!getopt(option, optArg)) {
    LOG4CXX_FATAL_FMT(logger, "Option -{} is missing\n\nUsage:\n{}\nWhere:\n{}",
                      option, synopsis(), description());
    exit(1);
  }
  std::istringstream iss(optArg);
  int val;
  iss >> val;
  if (!iss) {
    LOG4CXX_FATAL_FMT(logger,
                      "Option -{} has incorrect parameter (it is not an "
                      "integer)\n\nUsage:\n{}\nWhere:\n{}",
                      option, synopsis(), description());
    exit(1);
  }
  return val;
}
}  // namespace fbae::core
