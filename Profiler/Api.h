#ifndef _PROFILER_API_H
#define _PROFILER_API_H

#include <Profiler/Clock.h>
#include <Profiler/Config.h>
#include <Profiler/Decoder.h>
#include <Profiler/Instrumentation.h>
#include <Profiler/Writer/Processor.h>
#include <utility>

namespace Profiler {

using DefaultConfig = Config<Record::RecordStorageTypes, Mpl::TypeList<Writer::WriteToFile> >;

template <typename RecordList_, typename WriterList_>
void setup(Config<RecordList_, WriterList_> config_) {
  Control::setManager(config_);
  Control::getManager().startWriter();
  // append Record::RecordStorageTypes to RecordList_
}
// TODO: Use config stored on the Manager
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
