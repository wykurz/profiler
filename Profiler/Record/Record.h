#ifndef RECORD_RECORD_H
#define RECORD_RECORD_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Exception/Exception.h>
#include <Profiler/Instrumentation/Time.h>
#include <istream>
#include <ostream>

namespace Profiler {
namespace Record {

struct TimeRecord {
  using TimePoint = Time::Rdtsc::TimePoint;
  TimeRecord(const char *name_) : _name(name_), _t0(Time::Rdtsc::now()) {
    PROFILER_ASSERT(name_);
    _depth = _threadDepth++;
    _seqNum = _threadSeqNum++;
  }
  void finish() {
    PROFILER_ASSERT(1 <= _threadDepth);
    --_threadDepth;
    _t1 = Time::Rdtsc::now();
  }
  static void preamble(std::ostream &out_);
  static void decode(std::istream &in_, std::ostream &out_);
  bool dirty() const { return nullptr != _name; }
  friend std::ostream &operator<<(std::ostream &, const TimeRecord &);

private:
  static thread_local std::size_t _threadDepth;
  static thread_local std::size_t _threadSeqNum;
  const char *_name = nullptr;
  TimePoint _t0;
  TimePoint _t1;
  std::size_t _depth;
  std::size_t _seqNum;
};

inline std::ostream &operator<<(std::ostream &out_, const TimeRecord &record_) {
  Algorithm::encodeString(out_, record_._name);
  out_ << record_._t0 << record_._t1;
  Algorithm::encode(out_, record_._depth);
  Algorithm::encode(out_, record_._seqNum);
  return out_;
}

using NativeRecords = Mpl::TypeList<Record::TimeRecord>;
}
}

#endif
