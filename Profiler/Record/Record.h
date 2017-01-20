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
        using TimePoint = Time::Rdtsc::TimePoint;
        TimeRecord() = default;
        TimeRecord(const char* name_, TimePoint t0_, TimePoint t1_)
          : _name(name_),
            _t0(std::move(t0_)),
            _t1(std::move(t1_))
        { }
        static void decode(std::istream& in_, std::ostream& out_)
        {
            while (in_.good() && in_.peek() != EOF) {
                DLOG("Loop in TimeRecord decode, currently at: " << in_.tellg());
                auto name = Algorithm::decodeString(in_);
                TimePoint t0;
                TimePoint t1;
                in_ >> t0 >> t1;
                // TODO: JSON this
                out_ << "TimeRecord(" << name << ")[[" << t0.data << ", " << t1.data << "]]\n";
            }
        }
        friend std::ostream& operator<<(std::ostream&, const TimeRecord&);
      private:
        const char* _name = nullptr;
        TimePoint _t0;
        TimePoint _t1;
    };

    inline std::ostream& operator<<(std::ostream& out_, const TimeRecord& record_)
    {
        Algorithm::encodeString(out_, record_._name);
        out_ << record_._t0 << record_._t1;
        return out_;
    }

}
}

#endif
