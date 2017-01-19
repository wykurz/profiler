#ifndef INSTRUMENTATION_TIME_H
#define INSTRUMENTATION_TIME_H

#include <Profiler/Algorithm/Stream.h>
#include <chrono>
#include <ratio>
#include <iostream>

namespace Profiler { namespace Time
{

    struct TimeDelta
    {
        TimeDelta() = default;
        TimeDelta(std::chrono::duration<double> duration_)
          : _duration(std::move(duration_))
        { }
        TimeDelta(std::size_t nanos_)
          : _duration(std::chrono::nanoseconds(nanos_))
        { }
        TimeDelta(const TimeDelta& other_) = default;
        TimeDelta& operator=(const TimeDelta& other_) = default;
        TimeDelta(TimeDelta&& other_) = default;
        std::size_t nanos() const
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(_duration).count();
        }
      private:
        std::chrono::duration<double> _duration;
    };

    inline std::ostream& operator<<(std::ostream& out_, const TimeDelta& tdelta_)
    {
        auto nanos = tdelta_.nanos();
        out_.write(reinterpret_cast<const char*>(&nanos), sizeof(nanos));
        return out_;
    }

    inline std::istream& operator>>(std::istream& in_, TimeDelta& tdelta_)
    {
        auto nanos = Algorithm::decode<std::size_t>(in_);
        tdelta_ = TimeDelta(nanos);
        return in_;
    }

    template <typename ClockType_>
    struct Time
    {
        using ClockType = ClockType_;
        using TimePoint = std::chrono::time_point<ClockType>;
        TimeDelta delta() const
        {
            return TimeDelta(ClockType::now() - start);
        }
      private:
        const TimePoint start = ClockType::now();
    };

    using HighResTime = Time<std::chrono::high_resolution_clock>;

    struct RdtscClock
    {
        using rep = unsigned long long;
        // Determined using lscpu:
        using period = std::ratio<1, 3298092000>;
        using duration = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<RdtscClock>;
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
}

#endif
