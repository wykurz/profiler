#ifndef _PROFILER_API_H
#define _PROFILER_API_H

#include <Profiler/Clock.h>
#include <Profiler/Config.h>
#include <Profiler/Decoder.h>
#include <Profiler/Instrumentation.h>
#include <utility>

namespace Profiler {

inline void setup(const Config &config_) { Config::setConfig(config_); }
template <typename RecordTypes_ = Mpl::TypeList<>> void primeThisThread() {
  Profiler::Control::primeThisThread<Record::RecordStorageTypes>();
  Profiler::Control::primeThisThread<RecordTypes_>();
}
inline void stopWriter() { Profiler::Control::getManager().stopWriter(); }
template <typename Clock_, typename... Args_>
auto eventRecord(Args_ &&... args_) {
  return Instrumentation::eventRecord<Clock_>(std::forward<Args_>(args_)...);
}
} // namespace Profiler

#endif
