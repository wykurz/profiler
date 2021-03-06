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

struct Globals {
  static decltype(auto) getManager() { return Control::getManager(); }
  template <typename StorageType> static decltype(auto) getThreadRecords() {
    return Control::getThreadRecords<StorageType>();
  }
};

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

template <typename Clock_, typename Record_, typename Globals_ = Globals>
Record::EventId<Clock_> genEventId() {
  return {Globals_::getManager().id(),
          Globals_::template getThreadRecords<Record_>().id()};
}
} // namespace Internal

template <typename Clock_, typename Globals_ = Internal::Globals>
struct ProfilerScope {
  using RecordType = Record::ScopeRecord<Clock_>;
  using StorageType = typename RecordType::Storage;
  explicit ProfilerScope(const char *name_) : _record(name_) {}
  ~ProfilerScope() {
    Internal::doRecord(
        Globals_::template getThreadRecords<StorageType>().getRecordManager(),
        _record.finish());
  }
  ProfilerScope(const ProfilerScope &) = delete;

private:
  RecordType _record;
};

template <typename Clock_, typename Globals_ = Internal::Globals>
void eventRecord(const char *name_, Record::EventId<Clock_> eventId_) {
  using RecordType = Record::EventRecord<Clock_>;
  Internal::doRecord(
      Globals_::template getThreadRecords<RecordType>().getRecordManager(),
      RecordType(name_, eventId_));
}

template <typename Clock_, typename Globals_ = Internal::Globals>
auto eventRecord(const char *name_) {
  using RecordType = Record::EventRecord<Clock_>;
  auto eventId = Internal::genEventId<Clock_, RecordType>();
  auto record = RecordType(name_, eventId);
  Internal::doRecord(
      Globals_::template getThreadRecords<RecordType>().getRecordManager(),
      std::move(record));
  return eventId;
}

} // namespace Instrumentation
} // namespace Profiler

#endif
