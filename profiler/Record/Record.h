#ifndef RECORD_RECORD_H
#define RECORD_RECORD_H

#include <Queue/Queue.h>
#include <array>
#include <chrono>
#include <ostream>

namespace Profiler { namespace Record
{

    using TimeDelta = std::chrono::duration<double>;

    // TODO: For different record types will need to provide some kind of registration to distinguish after serialized
    struct Record
    {
        Record() = default;
        Record(const char* name_, const TimeDelta& delta_)
          : name(name_),
            delta(delta_)
        { }
        const char* name = nullptr;
        TimeDelta delta;
    };

    inline std::ostream& operator<<(std::ostream& out_, const Record& record_)
    {
        out_ << record_.name << record_.delta.count();
        return out_;
    }

    // TODO: Add a type list type that has all records available
    //       We'll need a for_each functionality...

}
}

#endif
