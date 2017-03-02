#ifndef _PROFILER_INSTRUMENTATION_TIME_H
#define _PROFILER_INSTRUMENTATION_TIME_H

#include <Profiler/Algorithm/Stream.h>
#include <chrono>
#include <iostream>
#include <ratio>
#include <x86intrin.h>

namespace Profiler {
namespace Instrumentation {

struct Rdtsc {
  struct TimePoint {
    using Storage = decltype(__rdtsc());
    Storage data;
  };
  static TimePoint now() { return {__rdtsc()}; }
};

inline std::ostream &operator<<(std::ostream &out_,
                                const Rdtsc::TimePoint &time_) {
  out_.write(reinterpret_cast<const char *>(&time_.data), sizeof(time_.data));
  return out_;
}

inline std::istream &operator>>(std::istream &in_, Rdtsc::TimePoint &time_) {
  auto data = Algorithm::decode<Rdtsc::TimePoint::Storage>(in_);
  time_ = Rdtsc::TimePoint{data};
  return in_;
}
} // namespace Instrumentation
} // namespace Profiler

#endif
