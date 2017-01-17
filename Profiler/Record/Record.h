#ifndef RECORD_RECORD_H
#define RECORD_RECORD_H

#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Queue/Queue.h>
#include <array>
#include <chrono>
#include <cstring>
#include <ostream>
#include <istream>
#include <typeindex>

namespace Profiler { namespace Record
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

    using TypeId = std::type_index;

    struct TimeRecord
    {
        TimeRecord() = default;
        TimeRecord(const char* name_, TimeDelta delta_)
          : _name(name_),
            _delta(std::move(delta_))
        { }
        static void decode(std::istream& in_, std::ostream& out_)
        {
            while (in_.good() && in_.peek() != EOF) {
                DLOG("Loop in TimeRecord decode, currently at: " << in_.tellg());
                auto name = Algorithm::decodeString(in_);
                TimeDelta tdelta;
                in_ >> tdelta;
                // TODO: JSON this
                out_ << "TimeRecord[" << name << ", " << tdelta.nanos() << "]\n";
            }
        }
        friend std::ostream& operator<<(std::ostream&, const TimeRecord&);
      private:
        const char* _name = nullptr;
        TimeDelta _delta;
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
