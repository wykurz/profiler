#ifndef _PROFILER_RECORD_RDTSC_ASYNC_RECORD_H
#define _PROFILER_RECORD_RDTSC_ASYNC_RECORD_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Exception.h>
#include <Profiler/Instrumentation/Time.h>
#include <Profiler/Log.h>
#include <Profiler/Record/RdtscRecordCommon.h>
#include <atomic>
#include <chrono>
#include <istream>
#include <ostream>
#include <thread>
#include <utility>

namespace Profiler {
namespace Record {

enum class AsyncType : bool { Start = false, Stop };

template <AsyncType asyncType_> struct RdtscAsyncRecord {
  using Rdtsc = Instrumentation::Rdtsc;
  using TimePoint = Rdtsc::TimePoint;
  RdtscAsyncRecord(const char *name_, std::size_t uniqueId_)
      : _name(name_), _uniqueId(uniqueId_), _time(Rdtsc::now()) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  static void preamble(std::ostream &out_) {
    Algorithm::encodeString(out_, Control::getManager().name().c_str());
    rdtscPreamble(out_);
  }
  static void decode(std::istream &in_, std::ostream &out_) {
    decodeRdtscReference(in_, out_);
    // TODO(mateusz)...
    out_ << "records:\n";
    auto async = asyncType_ == AsyncType::Start ? "start" : "stop";
    while (in_.good() && in_.peek() != EOF) {
      DLOG("Loop in RdtscScopeRecordStart decode, currently at: "
           << in_.tellg());
      auto name = Algorithm::decodeString(in_);
      auto id = Algorithm::decode<std::size_t>(in_);
      TimePoint time;
      in_ >> time;
      out_ << "- name: \"" << name << "\"\n";
      out_ << "  async: \"" << async << "\"\n";
      out_ << "  id: " << id << "\n";
      out_ << "  time: " << time.data << "\n";
    }
  }

  bool dirty() const { return nullptr != _name; }
  friend std::ostream &operator<<(std::ostream & /*out_*/,
                                  const RdtscAsyncRecord & /*record_*/);

protected:
  const char *const _name;
  const std::size_t _uniqueId;
  TimePoint _time;
};

template <AsyncType asyncType_>
inline std::ostream &operator<<(std::ostream &out_,
                                const RdtscAsyncRecord<asyncType_> &record_) {
  Algorithm::encodeString(out_, record_._name);
  Algorithm::encode(out_, record_._uniqueId);
  out_ << record_._time;
  return out_;
}

// TODO: This must be defined in single place
// using NativeRecords = Mpl::TypeList<Record::RdtscAsyncRecord>;
} // namespace Record
} // namespace Profiler

#endif
