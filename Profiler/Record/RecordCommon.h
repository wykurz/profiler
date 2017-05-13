#ifndef _PROFILER_RECORD_RDTSCRECORDCOMMON_H
#define _PROFILER_RECORD_RDTSCRECORDCOMMON_H

#include <Profiler/Clock.h>
#include <Profiler/Log.h>
#include <Profiler/Serialize.h>
#include <atomic>
#include <chrono>
#include <istream>
#include <ostream>
#include <thread>
#include <utility>

namespace Profiler {
namespace Record {


template <typename Clock_>
struct Preamble;

template <>
struct Preamble<Clock::Rdtsc>
{
  static void encode(std::ostream &out_) {
    // Measure:
    auto hiResNow = std::chrono::high_resolution_clock::now();
    auto rdtscNow = Clock::Rdtsc::now();
    // Serialize:
    auto nanosecondDuration = [](const auto &duration_) {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(duration_)
      .count();
    };
    std::uint64_t timeReference = nanosecondDuration(hiResNow.time_since_epoch());
    Serialize::encode(out_, timeReference);
    out_ << rdtscNow;
  }
  static void decode(std::istream &in_, std::ostream &out_) {
    auto timeReference = Serialize::decode<std::uint64_t>(in_);
    out_ << "time_reference:\n";
    out_ << "- time: " << timeReference << "\n";
    Clock::Rdtsc::TimePoint rdtscBase;
    in_ >> rdtscBase;
    out_ << "- rdtsc: " << rdtscBase.data << "\n";
  }
};
} // namespace Record
} // namespace Profiler

#endif
