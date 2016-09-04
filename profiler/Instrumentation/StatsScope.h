#ifndef INSTRUMENTATION_STATSSCOPE_H
#define INSTRUMENTATION_STATSSCOPE_H

#include <Control/RecordManager.h>
#include <Control/Thread.h>
#include <Instrumentation/Time.h>
#include <Record/Record.h>

namespace Scope
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

// TODO: this defines a temp. variable, we need to name it!
#define STATS_SCOPE() Scope::StatsScope(Control::getThread().template getRecordManager<Record::Record>(), \
                                        __PRETTY_FUNCTION__)

#endif
