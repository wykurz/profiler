#ifndef _PROFILER_CONFIG_CONFIG_H
#define _PROFILER_CONFIG_CONFIG_H

#include <Profiler/Exception.h>
#include <chrono>
#include <functional>
#include <string>
#include <tuple>

namespace Profiler {

struct ConfigBase {
  /**
   * Instance id is used when synchronizing event records across multiple
   * processes.
   */
  std::size_t instanceId = 0;
  // TODO(mateusz): Settings specifying the log should be used to instantiate
  // writer
  std::string binaryLogPrefix = ".cxxperf-log";
  std::string binaryLogDir = ".";
  std::string yamlLogName = "cxxperf-log.yaml";
  std::size_t arenaSize = std::size_t(100) * 1024 * 1024; // 100MiB
  // TODO(mateusz): Writer should auto-tune this value.
  std::chrono::microseconds writerSleepTime{100000};
};

template <typename RecordList_, typename... Writers_>
struct Config : ConfigBase {
  using RecordList = RecordList_;
  explicit Config(Writers_&&... writers_)
      : writers(std::move(writers_)...)
    {}
  std::tuple<Writers_...> writers;
};

template <typename RecordList_, typename... Writers_>
auto GetConfig(Writers_&&... writers_) {
  return Config<RecordList_, Writers_...>(std::move(writers_)...);
}
} // namespace Profiler

#endif
