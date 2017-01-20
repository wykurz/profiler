#ifndef INSTRUMENTATION_TIME_H
#define INSTRUMENTATION_TIME_H

#include <Profiler/Algorithm/Stream.h>
#include <chrono>
#include <ratio>
#include <iostream>

namespace Profiler { namespace Time
{

    struct Rdtsc
    {
        struct TimePoint
        {
            using Storage = std::uint64_t;
            Storage data;
        };
        static TimePoint now()
        {
            unsigned lo, hi;
            asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
            return {static_cast<TimePoint::Storage>(hi) << 32 | lo};
        }
    };

    inline std::ostream& operator<<(std::ostream& out_, const Rdtsc::TimePoint& time_)
    {
        out_.write(reinterpret_cast<const char*>(&time_.data), sizeof(time_.data));
        return out_;
    }

    inline std::istream& operator>>(std::istream& in_, Rdtsc::TimePoint& time_)
    {
        auto data = Algorithm::decode<Rdtsc::TimePoint::Storage>(in_);
        time_ = Rdtsc::TimePoint{data};
        return in_;
    }

}
}

#endif
