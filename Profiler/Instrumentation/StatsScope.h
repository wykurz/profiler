#ifndef _PROFILER_INSTRUMENTATION_STATSSCOPE_H
#define _PROFILER_INSTRUMENTATION_STATSSCOPE_H

#include <Profiler/Control/RecordManager.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Instrumentation/StatsScope.h>
#include <Profiler/Instrumentation/Time.h>
#include <Profiler/Log/Log.h>
#include <Profiler/Record/Record.h>

namespace Profiler {
namespace Scope {

template <typename Record_>
void record(Control::RecordManager<Record_> &recordManager_,
            Record_ &&record_) {
  auto record = recordManager_.getRecord();
  if (!record) {
    DLOG("No valid record!");
    return;
  }
  *record = std::forward<Record_>(record_);
}

struct StatsScope {
  explicit StatsScope(const char *name_) : _name(name_) {}
  ~StatsScope() {
    _record.finish();
    record(Control::getThreadRecords<Record::TimeRecord>().getRecordManager(),
           std::move(_record));
  }

private:
  const char *_name;
  Record::TimeRecord _record{_name};
};
} // namespace Scope
} // namespace Profiler

#ifndef NO_MACROS

#define _CAT(a, b) _CAT_I(a, b)
#define _CAT_I(a, b) _CAT_II(~, a##b)
#define _CAT_II(p, res) res
#define _UNIQUE_NAME(base) _CAT(base, __COUNTER__)

#define STATS_SCOPE_EX(name)                                                   \
  Profiler::Scope::StatsScope _UNIQUE_NAME(statsScope)(name)
#define STATS_SCOPE() STATS_SCOPE_EX(__PRETTY_FUNCTION__)

#endif // NO_MACROS

#endif
