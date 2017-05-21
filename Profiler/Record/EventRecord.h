#ifndef _PROFILER_RECORD_EVENT_RECORD_H
#define _PROFILER_RECORD_EVENT_RECORD_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Clock.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Exception.h>
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

template <typename Clock_> struct EventId {
  std::size_t instanceId;
  std::size_t recorderId;
};

template <typename Clock_>
std::ostream &operator<<(std::ostream &out_, const EventId<Clock_> &eventId_) {
  Serialize::encode(out_, eventId_.instanceId);
  Serialize::encode(out_, eventId_.recorderId);
  return out_;
}

template <typename Clock_>
std::istream &operator>>(std::istream &in_, EventId<Clock_> &eventId_) {
  eventId_.instanceId = Serialize::decode<std::size_t>(in_);
  eventId_.recorderId = Serialize::decode<std::size_t>(in_);
  return in_;
}

template <typename Clock_> struct EventRecord {
  using Clock = Clock_;
  using This = EventRecord<Clock_>;
  using TimePoint = typename Clock::TimePoint;
  using Duration = typename Clock::Duration;
  explicit EventRecord(const char *name_, EventId<Clock> eventId_)
      : _name(name_), _eventId(std::move(eventId_)), _time(Clock::now()) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  explicit EventRecord(const char *name_)
      : EventRecord(name_, {Control::getManager().id(), Control::getThreadRecords<This>().id})
    { }
  EventId<Clock_> eventId() const { return _eventId; }
  static void encodePreamble(std::ostream &out_) {
    Serialize::encode(out_, Control::getManager().id());
  }
  void encode(std::ostream &out_) {
    Serialize::encodeString(out_, _name);
    out_ << _eventId;
    out_ << _time;
  }
  static void decodePreamble(std::istream &in_, std::ostream &out_) {
    auto instanceId = Serialize::decode<std::size_t>(in_);
    out_ << "instance: " << instanceId << "\n";
  }
  static void decode(std::istream &in_, std::ostream &out_) {
    DLOG("Loop in ScopeRecordStart decode, currently at: " << in_.tellg());
    auto name = Serialize::decodeString(in_);
    EventId<Clock> eventId;
    in_ >> eventId;
    Duration duration;
    in_ >> duration;
    out_ << "- name: " << name << "\n";
    out_ << "  event_id:\n";
    out_ << "    instance: " << eventId.instanceId << "\n";
    out_ << "    recorder: " << eventId.recorderId << "\n";
    out_ << "  time: " << duration << "\n";
  }
  bool dirty() const { return nullptr != _name; }

protected:
  const char *_name;
  EventId<Clock> _eventId;
  TimePoint _time;
};
} // namespace Record
} // namespace Profiler

#endif
