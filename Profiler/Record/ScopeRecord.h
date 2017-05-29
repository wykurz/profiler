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
  using Duration = typename Clock::Duration;
  ScopeStorage(const char *name_, TimePoint t0_, TimePoint t1_,
               std::size_t depth_, std::size_t seqNum_)
      : _name(name_), _t0(std::move(t0_)), _t1(std::move(t1_)), _depth(depth_),
        _seqNum(seqNum_) {}
  static void encodePreamble(std::ostream & /*out_*/) {}
  void encode(std::ostream &out_) {
    Serialize::encodeString(out_, _name);
    out_ << _t0 << _t1;
    Serialize::encode(out_, _depth);
    Serialize::encode(out_, _seqNum);
  }
  static void decodePreamble(std::istream & /*in_*/, std::ostream & /*out_*/) {}
  static void decode(std::istream &in_, std::ostream &out_) {
    auto name = Serialize::decodeString(in_);
    Duration t0;
    Duration t1;
    in_ >> t0 >> t1;
    auto depth = Serialize::decode<std::size_t>(in_);
    auto seqNum = Serialize::decode<std::size_t>(in_);
    out_ << "- seq: " << seqNum << "\n";
    out_ << "  name: \"" << name << "\"\n";
    out_ << "  t0: " << t0 << "\n";
    out_ << "  t1: " << t1 << "\n";
    out_ << "  depth: " << depth << "\n";
  }
  bool dirty() const { return nullptr != _name; }

private:
  const char *_name;
  TimePoint _t0;
  TimePoint _t1;
  std::size_t _depth;
  std::size_t _seqNum;
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
