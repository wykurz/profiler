#ifndef _PROFILER_RECORD_EVENT_RECORD_H
#define _PROFILER_RECORD_EVENT_RECORD_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Clock.h>
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
  using TimePoint = typename Clock::TimePoint;
  EventRecord(const char *name_, EventId<Clock> eventId_)
      : name(name_), eventId(std::move(eventId_)), time(Clock::now()) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  bool dirty() const { return nullptr != name; }
  const char *name;
  EventId<Clock> eventId;
  TimePoint time;
};
} // namespace Record
} // namespace Profiler

#endif
