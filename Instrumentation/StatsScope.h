#include <Instrumentation/Time.h>

namespace Scope
{

    struct StatsScope
    {
        StatsScope(const char* name_)
          : _name(name_)
        { }

        ~StatsScope()
        {
            record();
        }

        void record()
        {
            std::cerr << "Name: " << _name << ", delta: " << _time.delta().count() << "\n";
        }

      private:
        const char* _name;
        const Time::RdtscTime _time = Time::RdtscTime();
    };

}

#define STATS_SCOPE() Scope::StatsScope(__PRETTY_FUNCTION__)
