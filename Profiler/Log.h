#ifndef _PROFILER_LOG_LOG_H
#define _PROFILER_LOG_LOG_H

#ifdef DEBUG_LOG

#include <Profiler/Defines.h>
#include <iostream>
#include <mutex>

namespace Profiler {
namespace Log {

inline std::mutex &logMutex() {
  static std::mutex mutex;
  return mutex;
}

} // namespace Log
} // namespace Profiler

#define DLOG(x)                                                                \
  do {                                                                         \
    std::unique_lock<std::mutex> lk(Profiler::Log::logMutex());                \
    std::cerr << __FILE__ << "(" << __LINE__ << "): " << PROFILER_FUNC_NAME     \
              << " :: " << x << "\n";                                          \
  } while (false);
#else
#define DLOG(x)                                                                \
  do {                                                                         \
  } while (false);
#endif

#endif
