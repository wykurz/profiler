#ifndef RECORD_RECORD_H
#define RECORD_RECORD_H

#include <Profiler/Queue/Queue.h>
#include <array>
#include <chrono>
#include <cstring>
#include <ostream>
#include <istream>

namespace Profiler { namespace Record
{

    using TimeDelta = std::chrono::duration<double>;

    struct Record
    {
        Record() = default;
        Record(const char* name_, const TimeDelta& delta_)
          : name(name_),
            delta(delta_)
        { }
        const char* name = nullptr;
        TimeDelta delta;
        static void decode(std::istream& in_, std::ostream& out_)
        {
            // TODO
        }
    };

    inline std::ostream& operator<<(std::ostream& out_, const Record& record_)
    {
        out_ << strlen(record_.name) << record_.name << record_.delta.count();
        return out_;
    }

}
}

#endif
