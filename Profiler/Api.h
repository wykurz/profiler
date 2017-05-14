#ifndef _PROFILER_API_H
#define _PROFILER_API_H

#include <Profiler/Clock.h>
#include <Profiler/Config.h>
#include <Profiler/Decoder.h>
#include <Profiler/Instrumentation.h>

namespace Profiler {

inline void setup(const Config &config_) { Config::setConfig(config_); }
template <typename RecordTypes_ = Mpl::TypeList<>> void primeThisThread() {
  Profiler::Control::primeThisThread<Record::NativeRecords>();
  Profiler::Control::primeThisThread<RecordTypes_>();
}
inline void stopWriter() { Profiler::Control::getManager().stopWriter(); }
template <typename Clock_>
inline Record::AsyncId<Clock_> recordAsyncStart(const char *name_) {
  return Instrumentation::recordAsyncStart<Clock_>(name_);
}
template <typename Clock_>
inline void recordAsyncEnd(const char *name_,
                           Record::AsyncId<Clock_> asyncId_) {
  Instrumentation::recordAsyncEnd<Clock_>(name_, asyncId_);
}
} // namespace Profiler

#endif
