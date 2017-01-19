#ifndef RECORD_RECORD_H
#define RECORD_RECORD_H

#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Instrumentation/Time.h>
#include <Profiler/Queue/Queue.h>
#include <array>
#include <chrono>
#include <cstring>
#include <ostream>
#include <istream>
#include <typeindex>

namespace Profiler { namespace Record
{

    using TypeId = std::type_index;

    struct TimeRecord
    {
        TimeRecord() = default;
        TimeRecord(const char* name_, Time::TimeDelta delta_)
          : _name(name_),
            _delta(std::move(delta_))
        { }
        static void decode(std::istream& in_, std::ostream& out_)
        {
            while (in_.good() && in_.peek() != EOF) {
                DLOG("Loop in TimeRecord decode, currently at: " << in_.tellg());
                auto name = Algorithm::decodeString(in_);
                Time::TimeDelta tdelta;
                in_ >> tdelta;
                // TODO: JSON this
                out_ << "TimeRecord[" << name << ", " << tdelta.nanos() << "]\n";
            }
        }
        friend std::ostream& operator<<(std::ostream&, const TimeRecord&);
      private:
        const char* _name = nullptr;
        Time::TimeDelta _delta;
    };

    inline std::ostream& operator<<(std::ostream& out_, const TimeRecord& record_)
    {
        Algorithm::encodeString(out_, record_._name);
        out_ << record_._delta;
        return out_;
    }

}
}

#endif
