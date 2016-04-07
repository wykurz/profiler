#include <Instrumentation/StatsScope.h>
#include <Queue/Queue.h>

namespace Scope
{

    using TimeDelta = ScopeTime::TimeDelta;

    struct Record : Queue::Node
    {
        const char* _name;
        const TimeDelta delta;
    };

    void StatsScope::record()
    {
        getThread().queue.push(new Record{_name, _time.delta()});
    }

}
