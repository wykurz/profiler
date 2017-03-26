#ifndef _PROFILER_RECORD_RDTSCRECORDCOMMON_H
#define _PROFILER_RECORD_RDTSCRECORDCOMMON_H

#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Instrumentation/Time.h>
#include <Profiler/Log.h>
#include <atomic>
#include <chrono>
#include <istream>
#include <ostream>
#include <thread>
#include <utility>

namespace Profiler {
namespace Record {

inline void rdtscPreamble(std::ostream &out_) {
  // Measure:
  auto hiResNow = std::chrono::high_resolution_clock::now();
  auto rdtscNow = Instrumentation::Rdtsc::now();
  // Serialize:
  auto nanosecondDuration = [](const auto &duration_) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration_)
        .count();
  };
  std::uint64_t timeReference = nanosecondDuration(hiResNow.time_since_epoch());
  Algorithm::encode(out_, timeReference);
  out_ << rdtscNow;
}

inline void decodeRdtscReference(std::istream &in_, std::ostream &out_) {
  auto timeReference = Algorithm::decode<std::uint64_t>(in_);
  out_ << "time_reference:\n";
  out_ << "- time: " << timeReference << "\n";
  Instrumentation::Rdtsc::TimePoint rdtscBase;
  in_ >> rdtscBase;
  out_ << "- rdtsc: " << rdtscBase.data << "\n";
}

} // namespace Record
} // namespace Profiler

#endif
