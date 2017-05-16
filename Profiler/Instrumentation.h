#ifndef _PROFILER_INSTRUMENTATION_PROFILERRECORD_H
#define _PROFILER_INSTRUMENTATION_PROFILERRECORD_H

#include <Profiler/Control/RecordManager.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Defines.h>
#include <Profiler/Log.h>
#include <Profiler/Record/EventRecord.h>
#include <Profiler/Record/ScopeRecord.h>

namespace Profiler {
namespace Instrumentation {
namespace Internal {

template <typename Record_>
void doRecord(Control::RecordManager<Record_> &recordManager_,
              Record_ &&record_) {
  auto record = recordManager_.getRecord();
  if (!record) {
    DLOG("No valid record!");
    return;
  }
  *record = std::forward<Record_>(record_);
}
} // namespace Internal

template <typename Clock_> struct ProfilerScope {
  using RecordType = Record::ScopeRecord<Clock_>;
  using StorageType = typename RecordType::Storage;
  explicit ProfilerScope(const char *name_) : _record(name_) {}
  ~ProfilerScope() {
    Internal::doRecord(
        Control::getThreadRecords<StorageType>().getRecordManager(),
        _record.finish());
  }

private:
  RecordType _record;
};

template <typename Clock_, typename... Args_>
inline Record::AsyncId<Clock_> recordAsync(const char *name_, Args_... args_) {
  using RecordType = Record::EventRecord<Clock_>;
  auto record = RecordType(name_, std::forward<Args_...>(args_)...);
  auto asyncId = record.asyncId();
  Internal::doRecord(Control::getThreadRecords<RecordType>().getRecordManager(),
                     std::move(record));
  return asyncId;
}
} // namespace Instrumentation
} // namespace Profiler

#ifndef PROFILER_NO_MACROS

#define _CAT(a, b) _CAT_I(a, b)
#define _CAT_I(a, b) _CAT_II(~, a##b)
#define _CAT_II(p, res) res
#define _UNIQUE_NAME(base) _CAT(base, __COUNTER__)

#define PROFILER_RDTSC_SCOPE_EX(name_)                                         \
  Profiler::Instrumentation::ProfilerScope<Profiler::Clock::Rdtsc>             \
      _UNIQUE_NAME(statsScope)(name_)
#define PROFILER_RDTSC_SCOPE(ClockType_)                                       \
  PROFILER_RDTSC_SCOPE_EX(PROFILER_FUNCTION)

#define PROFILER_STEADY_SCOPE_EX(name_)                                        \
  Profiler::Instrumentation::ProfilerScope<Profiler::Clock::Steady>            \
      _UNIQUE_NAME(statsScope)(name_)
#define PROFILER_STEADY_SCOPE(ClockType_)                                      \
  PROFILER_STEADY_SCOPE_EX(PROFILER_FUNCTION)

#endif // PROFILER_NO_MACROS

#endif
