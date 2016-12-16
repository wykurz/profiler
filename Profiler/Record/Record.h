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
    };

    inline std::ostream& operator<<(std::ostream& out_, const Record& record_)
    {
        out_ << strlen(record_.name) << record_.name << record_.delta.count();
        return out_;
    }

    struct OutRecord
    {
        OutRecord(const std::string& name_, const TimeDelta& delta_)
          : name(name_),
            delta(delta_)
        { }
        // TODO: register here?
        // Use Record's type index?
        std::string name;
        TimeDelta delta;
    };

    // TODO
    inline std::istream& operator>>(std::istream& in_, OutRecord& record_)
    {
	return in_;
    }

}
}

#endif
