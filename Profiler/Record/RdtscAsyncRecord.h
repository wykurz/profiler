#ifndef _PROFILER_RECORD_RDTSC_ASYNC_RECORD_H
#define _PROFILER_RECORD_RDTSC_ASYNC_RECORD_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <Profiler/Clock.h>
#include <Profiler/Record/RdtscRecordCommon.h>
#include <Profiler/Serialize.h>
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
  Serialize::encode(out_, asyncId_.instanceId);
  Serialize::encode(out_, asyncId_.recorderId);
  return out_;
}

inline std::istream &operator>>(std::istream &in_, AsyncId &asyncId_) {
  asyncId_.instanceId = Serialize::decode<std::size_t>(in_);
  asyncId_.recorderId = Serialize::decode<std::size_t>(in_);
  return in_;
}

struct RdtscAsyncRecordStart {
  using Rdtsc = Clock::Rdtsc;
  using This = RdtscAsyncRecordStart;
  using TimePoint = Rdtsc::TimePoint;
  explicit RdtscAsyncRecordStart(const char *name_) : _name(name_) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  static void encodePreamble(std::ostream &out_) {
    Serialize::encode(out_, Control::getManager().id());
    rdtscPreamble(out_);
  }
  void encode(std::ostream &out_) {
    Serialize::encodeString(out_, _name);
    Serialize::encode(out_, _recorderId);
    out_ << _time;
  }
  static void decodePreamble(std::istream &in_, std::ostream &out_) {
    auto instanceId = Serialize::decode<std::size_t>(in_);
    out_ << "instance: " << instanceId << "\n";
    decodeRdtscReference(in_, out_);
  }
  static void decode(std::istream &in_, std::ostream &out_) {
    auto name = Serialize::decodeString(in_);
    auto recorderId = Serialize::decode<std::size_t>(in_);
    TimePoint time;
    in_ >> time;
    out_ << "- name: " << name << "\n";
    out_ << "  recorder: " << recorderId << "\n";
    out_ << "  rdtsc: " << time.data << "\n";
  }
  bool dirty() const { return nullptr != _name; }
  AsyncId asyncId() const { return {Control::getManager().id(), _recorderId}; }

protected:
  const char *_name;
  std::size_t _recorderId = Control::getThreadRecords<This>().id;
  TimePoint _time = Rdtsc::now();
};

struct RdtscAsyncRecordEnd {
  using Rdtsc = Clock::Rdtsc;
  using TimePoint = Rdtsc::TimePoint;
  RdtscAsyncRecordEnd(const char *name_, AsyncId asyncId_)
      : _name(name_), _asyncId(std::move(asyncId_)), _time(Rdtsc::now()) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  static void encodePreamble(std::ostream &out_) {
    Serialize::encode(out_, Control::getManager().id());
    rdtscPreamble(out_);
  }
  void encode(std::ostream &out_) {
    Serialize::encodeString(out_, _name);
    out_ << _asyncId;
    out_ << _time;
  }
  static void decodePreamble(std::istream &in_, std::ostream &out_) {
    auto instanceId = Serialize::decode<std::size_t>(in_);
    out_ << "instance: " << instanceId << "\n";
    decodeRdtscReference(in_, out_);
  }
  static void decode(std::istream &in_, std::ostream &out_) {
    DLOG("Loop in RdtscScopeRecordStart decode, currently at: " << in_.tellg());
    auto name = Serialize::decodeString(in_);
    AsyncId asyncId;
    in_ >> asyncId;
    TimePoint time;
    in_ >> time;
    out_ << "- name: " << name << "\n";
    out_ << "  async_id:\n";
    out_ << "    instance: " << asyncId.instanceId << "\n";
    out_ << "    recorder: " << asyncId.recorderId << "\n";
    out_ << "  rdtsc: " << time.data << "\n";
  }
  bool dirty() const { return nullptr != _name; }

protected:
  const char *_name;
  AsyncId _asyncId;
  TimePoint _time;
};
} // namespace Record
} // namespace Profiler

#endif
