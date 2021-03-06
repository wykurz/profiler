#ifndef _PROFILER_API_H
#define _PROFILER_API_H

#include <Profiler/Clock.h>
#include <Profiler/Config.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Instrumentation.h>
#include <Profiler/Writer/FileWriter.h>
#include <utility>

namespace Profiler {

template <typename RecordList_>
using FileWriter = Writer::FileWriter<RecordList_>;

inline auto GetDefaultConfig() { return Config<Record::RecordStorageTypes>(); }
template <typename RecordList_, typename... Writers_>
void setup(Config<RecordList_> &config_, Writers_ &&... writers_) {
  Control::setManager(config_, std::forward<Writers_>(writers_)...);
  Control::getManager().startProcessor();
  // append Record::RecordStorageTypes to RecordList_
}
// TODO(mateusz): Allow passing user-defined thread context
// TODO(mateusz): Should userContext be a void*?
template <typename RecordTypes_ = Record::RecordStorageTypes>
void primeThisThread(const std::string &userContext_ = "") {
  Control::primeThisThread<RecordTypes_>(userContext_);
}
inline void stopProcessor() { Control::getManager().stopProcessor(); }
template <typename Clock_>
void eventRecord(const char *name_, Record::EventId<Clock_> eventId_) {
  Instrumentation::eventRecord<Clock_>(name_, eventId_);
}
template <typename Clock_> auto eventRecord(const char *name_) {
  return Instrumentation::eventRecord<Clock_>(name_);
}

template <typename Clock_> using Scope = Instrumentation::ProfilerScope<Clock_>;

using RdtscClock = Clock::Rdtsc;
using SteadyClock = Clock::Steady;
using SystemClock = Clock::System;
} // namespace Profiler

#endif
