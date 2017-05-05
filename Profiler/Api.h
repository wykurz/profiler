#ifndef _PROFILER_API_H
#define _PROFILER_API_H

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
inline Record::AsyncId recordAsyncStart(const char *name_) {
  return Instrumentation::recordAsyncStart(name_);
}
inline void recordAsyncEnd(const char *name_, Record::AsyncId asyncId_) {
  Instrumentation::recordAsyncEnd(name_, asyncId_);
}
} // namespace Profiler

#endif
