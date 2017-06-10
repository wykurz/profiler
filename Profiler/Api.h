#ifndef _PROFILER_API_H
#define _PROFILER_API_H

#include <Profiler/Clock.h>
#include <Profiler/Config.h>
#include <Profiler/Decoder.h>
#include <Profiler/Instrumentation.h>
#include <Profiler/Writer/Processor.h>
#include <utility>

namespace Profiler {

using DefaultConfig =
    Config<Record::RecordStorageTypes, Mpl::TypeList<Writer::FileWriter>>;

template <typename RecordList_, typename WriterList_>
void setup(Config<RecordList_, WriterList_> config_) {
  Control::setManager(config_);
  Control::getManager().startProcessor();
  // append Record::RecordStorageTypes to RecordList_
}
// TODO(mateusz): Use config stored on the Manager
template <typename RecordTypes_ = Mpl::TypeList<>> void primeThisThread() {
  Profiler::Control::primeThisThread<Record::RecordStorageTypes>();
  Profiler::Control::primeThisThread<RecordTypes_>();
}
inline void stopProcessor() { Profiler::Control::getManager().stopProcessor(); }
template <typename Clock_>
void eventRecord(const char *name_, Record::EventId<Clock_> eventId_) {
  Instrumentation::eventRecord<Clock_>(name_, eventId_);
}
template <typename Clock_>
auto eventRecord(const char *name_) {
  return Instrumentation::eventRecord<Clock_>(name_);
}

template <typename Clock_>
using Scope = Instrumentation::ProfilerScope<Clock_>;

using RdtscClock = Clock::Rdtsc;
using SteadyClock = Clock::Steady;
using SystemClock = Clock::System;
} // namespace Profiler

#endif
