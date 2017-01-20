#ifndef INSTRUMENTATION_STATSSCOPE_H
#define INSTRUMENTATION_STATSSCOPE_H

#include <Profiler/Control/RecordManager.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Instrumentation/Time.h>
#include <Profiler/Log/Log.h>
#include <Profiler/Record/Record.h>

namespace Profiler { namespace Scope
{

    template <typename Record_>
    void record(Control::RecordManager<Record_>& recordManager_, Record_&& record_)
    {
        auto holder = recordManager_.getRecord();
        if (!holder.isValid()) {
            DLOG("No valid holder!");
            return;
        }
        holder.getRecord() = std::forward<Record_>(record_);
    }

    struct StatsScope
    {
        using TimePoint = Time::Rdtsc::TimePoint;
        StatsScope(const char* name_)
          : _name(name_)
        { }
        ~StatsScope()
        {
            record(Control::getThreadRecords<Record::TimeRecord>().getRecordManager(),
                   Record::TimeRecord(_name, _t0, Time::Rdtsc::now()));
        }
      private:
        const char* _name;
        const TimePoint _t0{Time::Rdtsc::now()};
    };

}
}

#ifndef NO_MACROS

#define _CAT(a, b) _CAT_I(a, b)
#define _CAT_I(a, b) _CAT_II(~, a ## b)
#define _CAT_II(p, res) res
#define _UNIQUE_NAME(base) _CAT(base, __COUNTER__)

#define STATS_SCOPE() Profiler::Scope::StatsScope _UNIQUE_NAME(statsScope)(__PRETTY_FUNCTION__)

#endif // NO_MACROS

#endif
