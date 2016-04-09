#include <Instrumentation/StatsScope.h>
#include <Control/Manager.h>
#include <Queue/Queue.h>

namespace Scope
{

    using TimeDelta = ScopeTime::TimeDelta;

    struct Record : Queue::Node
    {
        Record(const char* name_, const TimeDelta& delta_)
          : name(name_),
            delta(delta_)
        { }
        const char* name;
        const TimeDelta delta;
    };

    void StatsScope::record()
    {
        Control::getThread().queue.push(new Record{_name, _time.delta()});
    }

}
