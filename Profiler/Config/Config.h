#ifndef _PROFILER_CONFIG_CONFIG_H
#define _PROFILER_CONFIG_CONFIG_H

#include <Profiler/Exception/Exception.h>
#include <string>

namespace Profiler {
namespace Config {

struct Config {
  explicit Config(std::string binaryLogPrefix_ = ".cxxperf-log",
                  std::string binaryLogDir_ = ".",
                  std::string yamlLogName_ = "cxxperf-log.yaml")
      : binaryLogPrefix(std::move(binaryLogPrefix_)),
        binaryLogDir(std::move(binaryLogDir_)),
        yamlLogName(std::move(yamlLogName_)) {}

  bool operator!=(const Config &other_) const {
    return binaryLogPrefix != other_.binaryLogPrefix or
           yamlLogName != other_.yamlLogName;
  }

  const std::string binaryLogPrefix;
  const std::string binaryLogDir;
  const std::string yamlLogName;
};

namespace Internal {

inline Config defaultConfig() { return Config(); }

inline const Config &doGetConfig(const Config &config_ = defaultConfig()) {
  static Config config(config_);
  return config;
}
} // namespace Internal

inline const Config &getConfig() { return Internal::doGetConfig(); }

inline const Config &setConfig(const Config &config_) {
  auto &config = Internal::doGetConfig(config_);
  if (config != config_)
    throw Exception::Runtime(
        "Config appears to have been already set to a different value");
  return config;
}
} // namespace Config
} // namespace Profiler

#endif
