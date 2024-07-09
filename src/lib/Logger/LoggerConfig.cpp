#include "Logger/LoggerConfig.h"

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/propertyconfigurator.h>

namespace fbae {

auto getLogger(const std::string& name) -> LoggerPtr {
  using namespace log4cxx;

  struct log4cxx_initializer {
    log4cxx_initializer() {
      if (PropertyConfigurator::configure("res/fbae_logger.properties") ==
          spi::ConfigurationStatus::NotConfigured)
        BasicConfigurator::configure();  // Send events to the console
    }
    ~log4cxx_initializer() { LogManager::shutdown(); }
    log4cxx_initializer(log4cxx_initializer const&) = delete;
    log4cxx_initializer& operator=(log4cxx_initializer const&) = delete;
  };

  static log4cxx_initializer initAndShutdown;

  return name.empty() ? LogManager::getRootLogger()
                      : LogManager::getLogger(name);
}
}  // namespace fbae