#ifndef RECORD_RECORD_H
#define RECORD_RECORD_H

#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Exception/Exception.h>
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

    struct TimeRecord
    {
        using TimePoint = Time::Rdtsc::TimePoint;
        TimeRecord(const char* name_, TimePoint t0_, TimePoint t1_)
          : _name(name_),
            _t0(std::move(t0_)),
            _t1(std::move(t1_))
        {
            PROFILER_ASSERT(name_);
        }

        static void preamble(std::ostream& out_)
        {
            // Measure:
            auto hiResNow = std::chrono::high_resolution_clock::now();
            auto rdtscNow = Time::Rdtsc::now();
            // Serialize:
            auto nanosecondDuration = [](const auto& duration_) {
                return std::chrono::duration_cast<std::chrono::nanoseconds>(duration_).count();
            };
            std::uint64_t timeReference = nanosecondDuration(hiResNow.time_since_epoch());
            Algorithm::encode(out_, timeReference);
            out_ << rdtscNow;
        }

        static void decode(std::istream& in_, std::ostream& out_)
        {
            auto timeReference = Algorithm::decode<std::uint64_t>(in_);
            out_ << "Time reference: " << timeReference << std::endl;
            Time::Rdtsc::TimePoint rdtscBase;
            in_ >> rdtscBase;
            out_ << "Rdtsc: " << rdtscBase.data << std::endl;
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
        bool dirty() const
        {
            return nullptr != _name;
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
