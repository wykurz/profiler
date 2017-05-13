#ifndef _PROFILER_INSTRUMENTATION_TIME_H
#define _PROFILER_INSTRUMENTATION_TIME_H

#include <Profiler/Serialize.h>
#include <Profiler/Intrinsics.h>
#include <chrono>
#include <istream>
#include <ostream>
#include <ratio>

namespace Profiler {
namespace Instrumentation {

struct Rdtsc {
  struct TimePoint {
    using Storage = decltype(Intrinsics::rdtsc());
    Storage data;
  };
  static TimePoint now() { return {Intrinsics::rdtsc()}; }
};

inline std::ostream &operator<<(std::ostream &out_,
                                const Rdtsc::TimePoint &time_) {
  out_.write(reinterpret_cast<const char *>(&time_.data), sizeof(time_.data));
  return out_;
}

inline std::istream &operator>>(std::istream &in_, Rdtsc::TimePoint &time_) {
  auto data = Serialize::decode<Rdtsc::TimePoint::Storage>(in_);
  time_ = Rdtsc::TimePoint{data};
  return in_;
}
} // namespace Instrumentation
} // namespace Profiler

#endif
