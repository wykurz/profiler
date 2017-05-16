#ifndef _PROFILER_RECORD_RECORDS_H
#define _PROFILER_RECORD_RECORDS_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Clock.h>
#include <Profiler/Record/AsyncRecord.h>
#include <Profiler/Record/ScopeRecord.h>

namespace Profiler {
namespace Record {

template <typename Clock_>
using RecordStorageTemplates =
  Mpl::TypeList<Record::AsyncRecord<Clock_>, typename Record::ScopeRecord<Clock_>::Storage>;

using RecordStorageTypes =
    Mpl::Concat<RecordStorageTemplates<Clock::Rdtsc>, RecordStorageTemplates<Clock::Steady>,
                RecordStorageTemplates<Clock::System>>::type;
} // namespace Record
} // namespace Profiler

#endif
