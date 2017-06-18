#ifndef _PROFILER_CONFIG_CONFIG_H
#define _PROFILER_CONFIG_CONFIG_H

#include <Profiler/Exception.h>
#include <chrono>
#include <functional>
#include <string>
#include <tuple>

namespace Profiler {

template <typename RecordList_> struct Config {
  using RecordList = RecordList_;
  /**
   * Instance id is used when synchronizing event records across multiple
   * processes.
   */
  std::size_t instanceId = 0;
  std::size_t arenaSize = std::size_t(100) * 1024 * 1024; // 100MiB
  // TODO(mateusz): Writer should auto-tune this value.
  std::chrono::microseconds writerSleepTime{100000};
};
} // namespace Profiler

#endif
