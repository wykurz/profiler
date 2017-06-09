#ifndef _PROFILER_CONFIG_CONFIG_H
#define _PROFILER_CONFIG_CONFIG_H

#include <Profiler/Exception.h>
#include <chrono>
#include <string>

namespace Profiler {

struct ConfigBase {
  /**
   * Instance id is used when synchronizing event records across multiple
   * processes.
   */
  std::size_t instanceId = 0;
  std::string binaryLogPrefix = ".cxxperf-log";
  std::string binaryLogDir = ".";
  std::string yamlLogName = "cxxperf-log.yaml";
  // TODO(mateusz): increase once we move to list FreeMap
  // TODO(mateusz): fix performance issue and increase
  // std::size_t arenaSize = std::size_t(5) * 1024 * 1024 * 1024; // 100MiB
  std::size_t arenaSize = std::size_t(5) * 1024 * 1024; // 100MiB
  // TODO(mateusz): Writer should auto-tune this value.
  std::chrono::microseconds writerSleepTime{100000};
};

template <typename RecordList_, typename WriterList_>
struct Config : ConfigBase {
  using RecordList = RecordList_;
  using WriterList = WriterList_; // TODO(mateusz): Best to provide a way to give set of types via a tuple of objects that will be used
};
} // namespace Profiler

#endif
