#include <iostream>
#include <chrono>

namespace Time {

    template <typename ClockType_>
    struct Time
    {
        using ClockType = ClockType_;
        using TimePoint = std::chrono::time_point<ClockType>;
        using TimeDelta = std::chrono::duration<double>;

        TimeDelta delta() const
        {
            return ClockType::now() - start;
        }

      private:
        const TimePoint start = ClockType::now();
    };

    using HighResTime = Time<std::chrono::high_resolution_clock>;

}
