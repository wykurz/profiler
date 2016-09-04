#ifndef INSTRUMENTATION_TIME_H
#define INSTRUMENTATION_TIME_H

#include <Record/Record.h>
#include <iostream>
#include <chrono>

namespace Time {

    template <typename ClockType_>
    struct Time
    {
        using ClockType = ClockType_;
        using TimePoint = std::chrono::time_point<ClockType>;

        Record::TimeDelta delta() const
        {
            return ClockType::now() - start;
        }

      private:
        const TimePoint start = ClockType::now();
    };

    using HighResTime = Time<std::chrono::high_resolution_clock>;

    struct RdtscClock
    {
        using rep = unsigned long long;
        // Determined using lscpu:
        typedef std::ratio<1, 3298097000> period;
        typedef std::chrono::duration<rep, period> duration;
        typedef std::chrono::time_point<RdtscClock> time_point;
        static const bool is_steady = true;

        static time_point now() noexcept
        {
            unsigned lo, hi;
            asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
            return time_point(duration(static_cast<rep>(hi) << 32 | lo));
        }
    };

    using RdtscTime = Time<RdtscClock>;

}

#endif
