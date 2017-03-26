#ifndef _PROFILER_RECORD_RDTSC_SCOPE_RECORD_H
#define _PROFILER_RECORD_RDTSC_SCOPE_RECORD_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Exception.h>
#include <Profiler/Instrumentation/Time.h>
#include <Profiler/Log.h>
#include <Profiler/Record/RdtscRecordCommon.h>
#include <atomic>
#include <chrono>
#include <istream>
#include <ostream>
#include <utility>

namespace Profiler {
namespace Record {

struct RdtscScopeRecord {
  using Rdtsc = Instrumentation::Rdtsc;
  using TimePoint = Rdtsc::TimePoint;
  explicit RdtscScopeRecord(const char *name_)
      : _name(name_), _t0(Rdtsc::now()) {
    PROFILER_ASSERT(name_);
    _depth = threadDepth()++;
    _seqNum = threadSeqNum()++;
    std::atomic_signal_fence(std::memory_order_acq_rel);
  }
  void finish() {
    std::atomic_signal_fence(std::memory_order_acq_rel);
    PROFILER_ASSERT(1 <= threadDepth());
    --threadDepth();
    _t1 = Rdtsc::now();
  }
  static void preamble(std::ostream &out_) { rdtscPreamble(out_); }
  static void decode(std::istream &in_, std::ostream &out_) {
    decodeRdtscReference(in_, out_);
    out_ << "records:\n";
    while (in_.good() && in_.peek() != EOF) {
      DLOG("Loop in RdtscScopeRecord decode, currently at: " << in_.tellg());
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
  bool dirty() const { return nullptr != _name; }
  friend std::ostream &operator<<(std::ostream & /*out_*/,
                                  const RdtscScopeRecord & /*record_*/);

private:
  static std::size_t &threadDepth() {
    thread_local std::size_t value;
    return value;
  }
  static std::size_t &threadSeqNum() {
    thread_local std::size_t value;
    return value;
  }
  const char *_name;
  TimePoint _t0;
  TimePoint _t1;
  std::size_t _depth;
  std::size_t _seqNum;
};

inline std::ostream &operator<<(std::ostream &out_,
                                const RdtscScopeRecord &record_) {
  Algorithm::encodeString(out_, record_._name);
  out_ << record_._t0 << record_._t1;
  Algorithm::encode(out_, record_._depth);
  Algorithm::encode(out_, record_._seqNum);
  return out_;
}

using NativeRecords = Mpl::TypeList<Record::RdtscScopeRecord>;
} // namespace Record
} // namespace Profiler

#endif
