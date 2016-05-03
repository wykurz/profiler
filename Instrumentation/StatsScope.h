#ifndef INSTRUMENTATION_STATSSCOPE_H
#define INSTRUMENTATION_STATSSCOPE_H

#include <Instrumentation/Time.h>

namespace Scope
{

    using ScopeTime = Time::RdtscTime;

    struct StatsScope
    {
        StatsScope(const char* name_)
          : _name(name_)
        { }
        ~StatsScope()
        {
            record();
        }
        void record();
      private:
        const char* _name;
        const ScopeTime _time = ScopeTime();
    };

}

#define STATS_SCOPE() Scope::StatsScope(__PRETTY_FUNCTION__)

#endif
