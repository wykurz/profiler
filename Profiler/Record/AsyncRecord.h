#ifndef _PROFILER_RECORD_ASYNC_RECORD_H
#define _PROFILER_RECORD_ASYNC_RECORD_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Clock.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <Profiler/Record/RecordCommon.h>
#include <Profiler/Serialize.h>
#include <atomic>
#include <chrono>
#include <istream>
#include <ostream>
#include <thread>
#include <utility>

namespace Profiler {
namespace Record {

template <typename Clock_> struct AsyncId {
  std::size_t instanceId;
  std::size_t recorderId;
};

template <typename Clock_>
std::ostream &operator<<(std::ostream &out_, const AsyncId<Clock_> &asyncId_) {
  Serialize::encode(out_, asyncId_.instanceId);
  Serialize::encode(out_, asyncId_.recorderId);
  return out_;
}

template <typename Clock_>
std::istream &operator>>(std::istream &in_, AsyncId<Clock_> &asyncId_) {
  asyncId_.instanceId = Serialize::decode<std::size_t>(in_);
  asyncId_.recorderId = Serialize::decode<std::size_t>(in_);
  return in_;
}

template <typename Clock_> struct AsyncRecordStart {
  using Clock = Clock_;
  using This = AsyncRecordStart;
  using TimePoint = typename Clock::TimePoint;
  using Duration = typename Clock::Duration;
  explicit AsyncRecordStart(const char *name_) : _name(name_) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  static void encodePreamble(std::ostream &out_) {
    Serialize::encode(out_, Control::getManager().id());
    Preamble<Clock>::encode(out_);
  }
  void encode(std::ostream &out_) {
    Serialize::encodeString(out_, _name);
    Serialize::encode(out_, _recorderId);
    out_ << _time;
  }
  static void decodePreamble(std::istream &in_, std::ostream &out_) {
    auto instanceId = Serialize::decode<std::size_t>(in_);
    out_ << "instance: " << instanceId << "\n";
    Preamble<Clock>::decode(in_, out_);
  }
  static void decode(std::istream &in_, std::ostream &out_) {
    auto name = Serialize::decodeString(in_);
    auto recorderId = Serialize::decode<std::size_t>(in_);
    Duration duration;
    in_ >> duration;
    out_ << "- name: " << name << "\n";
    out_ << "  recorder: " << recorderId << "\n";
    out_ << "  time: " << duration << "\n";
  }
  bool dirty() const { return nullptr != _name; }
  AsyncId<Clock> asyncId() const {
    return {Control::getManager().id(), _recorderId};
  }

protected:
  const char *_name;
  std::size_t _recorderId = Control::getThreadRecords<This>().id;
  TimePoint _time = Clock::now();
};

// TODO(mateusz): Make End a Cont (Continuation), meaning that we should be able to chain
// more than 2 async events: Start -> Cont -> Cont -> ... -> Cont
template <typename Clock_> struct AsyncRecordEnd {
  using Clock = Clock_;
  using TimePoint = typename Clock::TimePoint;
  using Duration = typename Clock::Duration;
  AsyncRecordEnd(const char *name_, AsyncId<Clock> asyncId_)
      : _name(name_), _asyncId(std::move(asyncId_)), _time(Clock::now()) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  static void encodePreamble(std::ostream &out_) {
    Serialize::encode(out_, Control::getManager().id());
    Preamble<Clock>::encode(out_);
  }
  void encode(std::ostream &out_) {
    Serialize::encodeString(out_, _name);
    out_ << _asyncId;
    out_ << _time;
  }
  static void decodePreamble(std::istream &in_, std::ostream &out_) {
    auto instanceId = Serialize::decode<std::size_t>(in_);
    out_ << "instance: " << instanceId << "\n";
    Preamble<Clock>::decode(in_, out_);
  }
  static void decode(std::istream &in_, std::ostream &out_) {
    DLOG("Loop in ScopeRecordStart decode, currently at: " << in_.tellg());
    auto name = Serialize::decodeString(in_);
    AsyncId<Clock> asyncId;
    in_ >> asyncId;
    Duration duration;
    in_ >> duration;
    out_ << "- name: " << name << "\n";
    out_ << "  async_id:\n";
    out_ << "    instance: " << asyncId.instanceId << "\n";
    out_ << "    recorder: " << asyncId.recorderId << "\n";
    out_ << "  time: " << duration << "\n";
  }
  bool dirty() const { return nullptr != _name; }

protected:
  const char *_name;
  AsyncId<Clock> _asyncId;
  TimePoint _time;
};

using RdtscAsyncRecordStart = AsyncRecordStart<Clock::Rdtsc>;
using RdtscAsyncRecordEnd = AsyncRecordEnd<Clock::Rdtsc>;

} // namespace Record
} // namespace Profiler

#endif
