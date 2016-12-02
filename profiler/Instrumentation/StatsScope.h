#ifndef INSTRUMENTATION_STATSSCOPE_H
#define INSTRUMENTATION_STATSSCOPE_H

#include <Control/RecordManager.h>
#include <Control/Thread.h>
#include <Instrumentation/Time.h>
#include <Record/Record.h>

namespace Profiler { namespace Scope
{

    using ScopeTime = Time::RdtscTime;

    struct StatsScope
    {
        using RecordManagerType = Control::RecordManager<Record::Record>;
        StatsScope(RecordManagerType& recordManager_, const char* name_)
          : _recordManager(recordManager_), _name(name_)
        { }
        ~StatsScope()
        {
            record();
        }
        void record();
      private:
        RecordManagerType& _recordManager;
        const char* _name;
        const ScopeTime _time = ScopeTime();
    };

}
}

#ifndef NO_MACROS

#define _CAT(a, b) _CAT_I(a, b)
#define _CAT_I(a, b) _CAT_II(~, a ## b)
#define _CAT_II(p, res) res
#define _UNIQUE_NAME(base) _CAT(base, __COUNTER__)

#define STATS_SCOPE()                                                                         \
    Profiler::Scope::StatsScope _UNIQUE_NAME(statsScope)(                                     \
        Profiler::Control::getThread().template getRecordManager<Profiler::Record::Record>(), \
        __PRETTY_FUNCTION__)

#endif // NO_MACROS

#endif
