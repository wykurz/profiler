#ifndef _PROFILER_RECORD_SCOPE_RECORD_H
#define _PROFILER_RECORD_SCOPE_RECORD_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Clock.h>
#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <Profiler/Serialize.h>
#include <atomic>
#include <chrono>
#include <istream>
#include <ostream>
#include <utility>

namespace Profiler {
namespace Record {

template <typename Clock_> struct ScopeStorage {
  using Clock = Clock_;
  using TimePoint = typename Clock::TimePoint;
  ScopeStorage(const char *name_, TimePoint t0_, TimePoint t1_,
               std::size_t depth_, std::size_t seqNum_)
      : name(name_), t0(std::move(t0_)), t1(std::move(t1_)), depth(depth_),
        seqNum(seqNum_) {}
  bool dirty() const { return nullptr != name; }
  const char *name;
  TimePoint t0;
  TimePoint t1;
  std::size_t depth;
  std::size_t seqNum;
};

struct ScopeRecordBase {
  ScopeRecordBase() {
    _depth = threadDepth()++;
    _seqNum = threadSeqNum()++;
  }
  ~ScopeRecordBase() {
    PROFILER_ASSERT(1 <= threadDepth());
    --threadDepth();
  }
  std::size_t depth() const { return _depth; }
  std::size_t seqNum() const { return _seqNum; }

private:
  static std::size_t &threadDepth() {
    thread_local std::size_t value;
    return value;
  }
  static std::size_t &threadSeqNum() {
    thread_local std::size_t value;
    return value;
  }
  std::size_t _depth;
  std::size_t _seqNum;
};

template <typename Clock_> struct ScopeRecord : ScopeRecordBase {
  using Clock = Clock_;
  using Storage = ScopeStorage<Clock>;
  using TimePoint = typename Clock::TimePoint;
  explicit ScopeRecord(const char *name_) : _name(name_), _t0(Clock::now()) {
    PROFILER_ASSERT(name_);
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  Storage finish() {
    std::atomic_signal_fence(std::memory_order_acq_rel);
    return {_name, _t0, Clock::now(), depth(), seqNum()};
  }

private:
  const char *_name;
  TimePoint _t0;
};
} // namespace Record
} // namespace Profiler

#endif
