#ifndef _PROFILER_RECORD_RECORDS_H
#define _PROFILER_RECORD_RECORDS_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Record/RdtscAsyncRecord.h>
#include <Profiler/Record/RdtscScopeRecord.h>

namespace Profiler {
namespace Record {

using NativeRecords = Mpl::TypeList<Record::RdtscAsyncRecordStart,
                                    Record::RdtscAsyncRecordEnd,
                                    Record::RdtscScopeRecord>;
} // namespace Record
} // namespace Profiler

#endif
