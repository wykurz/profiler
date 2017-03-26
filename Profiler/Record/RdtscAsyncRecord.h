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

struct AsyncId {
  std::size_t instanceId;
  std::size_t recorderId;
};

inline std::ostream &operator<<(std::ostream &out_, const AsyncId &asyncId_) {
  Algorithm::encode(out_, asyncId_.instanceId);
  Algorithm::encode(out_, asyncId_.recorderId);
  return out_;
}

inline std::istream &operator>>(std::istream &in_, AsyncId &asyncId_) {
  asyncId_.instanceId = Algorithm::decode<std::size_t>(in_);
  asyncId_.recorderId = Algorithm::decode<std::size_t>(in_);
  return in_;
}

struct RdtscAsyncRecordStart {
  using Rdtsc = Instrumentation::Rdtsc;
  using TimePoint = Rdtsc::TimePoint;
  RdtscAsyncRecordStart(const char *name_, std::size_t recorderId_)
      : _name(name_), _recorderId(recorderId_), _time(Rdtsc::now()) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  static void preamble(std::ostream &out_) {
    Algorithm::encode(out_, Control::getManager().id());
    rdtscPreamble(out_);
  }
  static void decode(std::istream &in_, std::ostream &out_) {
    auto instanceId = Algorithm::decode<std::size_t>(in_);
    out_ << "instance: " << instanceId << "\n";
    decodeRdtscReference(in_, out_);
    out_ << "records:\n";
    while (in_.good() && in_.peek() != EOF) {
      DLOG("Loop in RdtscScopeRecordStart decode, currently at: "
           << in_.tellg());
      auto name = Algorithm::decodeString(in_);
      auto recorderId = Algorithm::decode<std::size_t>(in_);
      TimePoint time;
      in_ >> time;
      out_ << "- name: " << name << "\n";
      out_ << "  recorder: " << recorderId << "\n";
      out_ << "  time: " << time.data << "\n";
    }
  }

  bool dirty() const { return nullptr != _name; }
  friend std::ostream &operator<<(std::ostream & /*out_*/,
                                  const RdtscAsyncRecordStart & /*record_*/);

protected:
  const char *const _name;
  const std::size_t _recorderId;
  TimePoint _time;
};

inline std::ostream &operator<<(std::ostream &out_,
                                const RdtscAsyncRecordStart &record_) {
  Algorithm::encodeString(out_, record_._name);
  Algorithm::encode(out_, record_._recorderId);
  out_ << record_._time;
  return out_;
}

struct RdtscAsyncRecordEnd {
  using Rdtsc = Instrumentation::Rdtsc;
  using TimePoint = Rdtsc::TimePoint;
  RdtscAsyncRecordEnd(const char *name_, AsyncId asyncId_)
      : _name(name_), _asyncId(asyncId_), _time(Rdtsc::now()) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  static void preamble(std::ostream &out_) { rdtscPreamble(out_); }
  static void decode(std::istream &in_, std::ostream &out_) {
    auto instanceId = Algorithm::decode<std::size_t>(in_);
    out_ << "instance: " << instanceId << "\n";
    decodeRdtscReference(in_, out_);
    out_ << "records:\n";
    while (in_.good() && in_.peek() != EOF) {
      DLOG("Loop in RdtscScopeRecordStart decode, currently at: "
           << in_.tellg());
      auto name = Algorithm::decodeString(in_);
      AsyncId asyncId;
      in_ >> asyncId;
      TimePoint time;
      in_ >> time;
      out_ << "- name: " << name << "\n";

      out_ << "  async_id:\n";
      out_ << "    instance: " << asyncId.instanceId << "\n";
      out_ << "    recorder: " << asyncId.recorderId << "\n";

      out_ << "  time: " << time.data << "\n";
    }
  }
  bool dirty() const { return nullptr != _name; }
  friend std::ostream &operator<<(std::ostream & /*out_*/,
                                  const RdtscAsyncRecordEnd & /*record_*/);

protected:
  const char *const _name;
  AsyncId _asyncId;
  TimePoint _time;
};

inline std::ostream &operator<<(std::ostream &out_,
                                const RdtscAsyncRecordEnd &record_) {
  Algorithm::encodeString(out_, record_._name);
  out_ << record_._asyncId;
  out_ << record_._time;
  return out_;
}

// TODO: This must be defined in single place
// using NativeRecords = Mpl::TypeList<Record::RdtscAsyncRecord>;
} // namespace Record
} // namespace Profiler

#endif
