#include <Control/Manager.h>
#include <Control/Store.h>
#include <Instrumentation/StatsScope.h>

namespace Scope
{

    using TimeDelta = ScopeTime::TimeDelta;

    struct Record
    {
        Record(const char* name_, const TimeDelta& delta_)
          : name(name_),
            delta(delta_)
        { }

        virtual ~Record() = default;

        const char* name;
        const TimeDelta delta;
    };

    // TODO: Should be somewhere else?
    void store(const Record& record_, std::ostream& out_)
    {
        // TODO: ...
    }

    void StatsScope::record()
    {
        Control::getThread().queue.push(new Control::Queue::NodeType(Record(_name, _time.delta())));
    }

}
