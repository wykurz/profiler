#ifndef _PROFILER_RECORD_RECORDS_H
#define _PROFILER_RECORD_RECORDS_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Clock.h>
#include <Profiler/Record/AsyncRecord.h>
#include <Profiler/Record/ScopeRecord.h>

namespace Profiler {
namespace Record {

template <typename Clock_>
using RecordTemplates = Mpl::TypeList<Record::AsyncRecord<Clock_>, Record::ScopeRecord<Clock_>>;

using NativeRecords = Mpl::Concat<RecordTemplates<Clock::Rdtsc>,
                                  RecordTemplates<Clock::Steady>>::type;
} // namespace Record
} // namespace Profiler

#endif
