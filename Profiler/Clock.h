#ifndef _PROFILER_INSTRUMENTATION_TIME_H
#define _PROFILER_INSTRUMENTATION_TIME_H

#include <Profiler/Intrinsics.h>
#include <Profiler/Serialize.h>
#include <chrono>
#include <istream>
#include <ostream>
#include <ratio>

namespace Profiler {
namespace Clock {

struct Rdtsc {
  struct TimePoint {
    using Storage = decltype(Intrinsics::rdtsc());
    Storage data;
  };
  struct Duration {
    using Storage = std::uint64_t;
    Storage data;
  };
  static TimePoint now() { return {Intrinsics::rdtsc()}; }
};

inline std::ostream &operator<<(std::ostream &out_,
                                const Rdtsc::TimePoint &time_) {
  std::uint64_t ticks = time_.data;
  Serialize::encode(out_, ticks);
  return out_;
}

inline std::istream &operator>>(std::istream &in_, Rdtsc::Duration &duration_) {
  auto ticks = Serialize::decode<std::uint64_t>(in_);
  duration_ = Rdtsc::Duration{ticks};
  return in_;
}

inline std::ostream &operator<<(std::ostream &out_,
                                const Rdtsc::Duration &duration_) {
  out_ << duration_.data;
  return out_;
}

struct Steady {
  struct TimePoint {
    using Storage = decltype(std::chrono::steady_clock::now());
    Storage data;
  };
  struct Duration {
    using Storage = std::chrono::nanoseconds;
    Storage data;
  };
  static TimePoint now() { return {std::chrono::steady_clock::now()}; }
};

inline std::ostream &operator<<(std::ostream &out_,
                                const Steady::TimePoint &time_) {
  std::uint64_t nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            time_.data.time_since_epoch())
                            .count();
  Serialize::encode(out_, nanos);
  return out_;
}

inline std::istream &operator>>(std::istream &in_,
                                Steady::Duration &duration_) {
  duration_.data =
      Steady::Duration::Storage{Serialize::decode<std::uint64_t>(in_)};
  return in_;
}

inline std::ostream &operator<<(std::ostream &out_,
                                const Steady::Duration &duration_) {
  out_ << duration_.data.count();
  return out_;
}
// TODO(mateusz): - add system clock!
} // namespace Clock
} // namespace Profiler

#endif
