#ifndef _PROFILER_CONFIG_CONFIG_H
#define _PROFILER_CONFIG_CONFIG_H

#include <string>

namespace Profiler {
namespace Config {

struct Config {
  explicit Config(std::string binaryLogPrefix_ = ".cxxperf-log",
         std::string binaryLogDir_ = ".",
         std::string yamlLogName_ = "cxxperf-log.yaml");
  bool operator!=(const Config &other_) const;
  const std::string binaryLogPrefix;
  const std::string binaryLogDir;
  const std::string yamlLogName;
};

const Config &getConfig();
const Config &setConfig(const Config &config_);
}  // namespace Config
}  // namespace Profiler

#endif
