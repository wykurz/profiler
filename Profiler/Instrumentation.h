#ifndef _PROFILER_INSTRUMENTATION_PROFILERRECORD_H
#define _PROFILER_INSTRUMENTATION_PROFILERRECORD_H

#include <Profiler/Control/Manager.h>
#include <Profiler/Control/RecordManager.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Defines.h>
#include <Profiler/Log.h>
#include <Profiler/Record/EventRecord.h>
#include <Profiler/Record/ScopeRecord.h>
#include <utility>

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

template <typename Clock_, typename Record_>
Record::EventId<Clock_> genEventId() {
  return {Control::getManager().id(),
          Control::getThreadRecords<Record_>().id()};
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
  ProfilerScope(const ProfilerScope &) = delete;

private:
  RecordType _record;
};

template <typename Clock_>
void eventRecord(const char *name_, Record::EventId<Clock_> eventId_) {
  using RecordType = Record::EventRecord<Clock_>;
  Internal::doRecord(Control::getThreadRecords<RecordType>().getRecordManager(),
                     RecordType(name_, eventId_));
}

template <typename Clock_> auto eventRecord(const char *name_) {
  using RecordType = Record::EventRecord<Clock_>;
  auto eventId = Internal::genEventId<Clock_, RecordType>();
  auto record = RecordType(name_, eventId);
  Internal::doRecord(Control::getThreadRecords<RecordType>().getRecordManager(),
                     std::move(record));
  return eventId;
}

} // namespace Instrumentation
} // namespace Profiler

#ifndef PROFILER_NO_MACROS

#define _CAT(a, b) _CAT_I(a, b)
#define _CAT_I(a, b) _CAT_II(~, a##b)
#define _CAT_II(p, res) res
#define _UNIQUE_NAME(base) _CAT(base, __COUNTER__)

#define PROFILER_SCOPE_EX(clock_, name_)                                       \
  Profiler::Instrumentation::ProfilerScope<Profiler::Clock::clock_>            \
      _UNIQUE_NAME(statsScope)(name_)
#define PROFILER_SCOPE(clock_) PROFILER_SCOPE_EX(clock_, PROFILER_FUNC_NAME)

#define PROFILER_EVENT_EX(clock_, name_, args...)                              \
  Profiler::Instrumentation::eventRecord<Profiler::Clock::clock_>(name_, ##args)
#define PROFILER_EVENT(clock_, args...)                                        \
  PROFILER_EVENT_EX(clock_, PROFILER_FUNC_NAME, ##args)

#endif // PROFILER_NO_MACROS

#endif
