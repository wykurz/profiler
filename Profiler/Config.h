#ifndef _PROFILER_CONFIG_CONFIG_H
#define _PROFILER_CONFIG_CONFIG_H

#include <Profiler/Exception.h>
#include <string>

namespace Profiler {

struct Config {
  Config() = default;
  Config(const Config &) = default;
  bool operator!=(const Config &other_) const {
    return instanceId != other_.instanceId ||
           binaryLogPrefix != other_.binaryLogPrefix ||
           binaryLogDir != other_.binaryLogDir ||
           yamlLogName != other_.yamlLogName;
  }
  static const Config &getConfig();
  static void setConfig(const Config &config_);

  /**
   * Instance id is used when synchronizing event records across multiple
   * processes.
   */
  std::size_t instanceId = 0;
  std::string binaryLogPrefix = ".cxxperf-log";
  std::string binaryLogDir = ".";
  std::string yamlLogName = "cxxperf-log.yaml";
};

namespace Internal {

inline Config defaultConfig() { return Config(); }

inline const Config &doGetConfig(const Config &config_ = defaultConfig()) {
  static const Config config(config_);
  return config;
}
} // namespace Internal

inline const Config &Config::getConfig() { return Internal::doGetConfig(); }

inline void Config::setConfig(const Config &config_) {
  auto &config = Internal::doGetConfig(config_);
  if (config != config_)
    throw Exception::Runtime(
        "Config appears to have been already set to a different value");
}
} // namespace Profiler

#endif
