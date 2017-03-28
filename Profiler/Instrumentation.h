#ifndef _PROFILER_INSTRUMENTATION_PROFILERRECORD_H
#define _PROFILER_INSTRUMENTATION_PROFILERRECORD_H

#include <Profiler/Control/RecordManager.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Defines.h>
#include <Profiler/Log.h>
#include <Profiler/Rdtsc.h>
#include <Profiler/Record/RdtscAsyncRecord.h>
#include <Profiler/Record/RdtscScopeRecord.h>

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

struct ProfilerScope {
  explicit ProfilerScope(const char *name_) : _record(name_) {}
  ~ProfilerScope() {
    _record.finish();
    Internal::doRecord(Control::getThreadRecords<Record::RdtscScopeRecord>()
                           .getRecordManager(),
                       std::move(_record));
  }

private:
  Record::RdtscScopeRecord _record;
};

inline Record::AsyncId recordAsyncStart(const char *name_) {
  using RecordType = Record::RdtscAsyncRecordStart;
  auto record = Record::RdtscAsyncRecordStart(name_);
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

#define PROFILER_SCOPE_EX(name)                                                \
  Profiler::Instrumentation::ProfilerScope _UNIQUE_NAME(statsScope)(name)
#define PROFILER_SCOPE() PROFILER_SCOPE_EX(PROFILER_FUNCTION)

#endif // PROFILER_NO_MACROS

#endif
