#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Exception/Exception.h>
#include <Profiler/Instrumentation/Time.h>
#include <Profiler/Log/Log.h>
#include <Profiler/Record/Record.h>
#include <chrono>
#include <istream>
#include <ostream>

namespace Profiler {
namespace Record {

void TimeRecord::preamble(std::ostream &out_) {
  // Measure:
  auto hiResNow = std::chrono::high_resolution_clock::now();
  auto rdtscNow = Time::Rdtsc::now();
  // Serialize:
  auto nanosecondDuration = [](const auto &duration_) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration_)
        .count();
  };
  std::uint64_t timeReference = nanosecondDuration(hiResNow.time_since_epoch());
  Algorithm::encode(out_, timeReference);
  out_ << rdtscNow;
}

void TimeRecord::decode(std::istream &in_, std::ostream &out_) {
  auto timeReference = Algorithm::decode<std::uint64_t>(in_);
  out_ << "time_reference:\n";
  out_ << "- time: " << timeReference << "\n";
  Time::Rdtsc::TimePoint rdtscBase;
  in_ >> rdtscBase;
  out_ << "- rdtsc: " << rdtscBase.data << "\n";
  out_ << "records:\n";
  while (in_.good() && in_.peek() != EOF) {
    DLOG("Loop in TimeRecord decode, currently at: " << in_.tellg());
    auto name = Algorithm::decodeString(in_);
    TimePoint t0;
    TimePoint t1;
    in_ >> t0 >> t1;
    auto depth = Algorithm::decode<std::size_t>(in_);
    auto seqNum = Algorithm::decode<std::size_t>(in_);
    // TODO(mateusz): JSON this
    out_ << "- seq: " << seqNum << "\n";
    out_ << "  name: \"" << name << "\"\n";
    out_ << "  t0: " << t0.data << "\n";
    out_ << "  t1: " << t1.data << "\n";
    out_ << "  depth: " << depth << "\n";
  }
}

thread_local std::size_t TimeRecord::_threadDepth = 0;
thread_local std::size_t TimeRecord::_threadSeqNum = 0;
} // namespace Record
} // namespace Profiler
