#include <chrono>
#include <unordered_map>
#include <boost/optional.hpp>

namespace PerfUtils
{

    struct StatsScope
    {
        using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;
        using Duration = std::chrono::duration<double>;

        struct Stats
        {
            double avg;
            double min;
            double max;
            double total;
        };

        struct Record
        {
            Duration totalTime;
            std::size_t callCount = 0;
            boost::optional<Stats> stats;
        };

        using Data = std::unordered_map<const char*, Record>;

        StatsScope(Data& data_, const char* scope_, std::size_t printFreq_ = 1)
            : _data(data_),
              _scope(scope_),
              _printFreq(printFreq_)
        {
        }

        ~StatsScope()
        {
            _record.totalTime += now() - _creationTime;
            _record.callCount += 1;
            print();
        }

        void setData(double x)
        {
            if (not _record.stats)
            {
                _record.stats = Stats({x, x, x, x});
                return;
            }
            Stats& s = _record.stats.get();
            double n = static_cast<double>(_record.callCount + 1);
            s.avg *= (n - 1) / n;
            s.avg += x / n;
            s.min = std::min(x, s.min);
            s.max = std::max(x, s.max);
            s.total += x;
        }

      private:
        void print() const
        {
            if (0 != _record.callCount % _printFreq)
            {
                return;
            }
            std::cerr << "Scope: " << _scope << '\n';
            std::cerr << "  totalTime: " << _record.totalTime.count() << '\n';
            std::cerr << "  callCount: " << _record.callCount << std::endl;
            if (_record.stats)
            {
                const Stats& s = _record.stats.get();
                std::cerr << "  stats.avg: " << s.avg << std::endl;
                std::cerr << "  stats.max: " << s.max << std::endl;
                std::cerr << "  stats.min: " << s.min << std::endl;
                std::cerr << "  stats.total: " << s.total << std::endl;
            }
        }

        static Time now()
        {
            return std::chrono::high_resolution_clock::now();
        }

        Data&             _data;
        const char*       _scope;
        const std::size_t _printFreq;
        Record&           _record = _data.insert({_scope, Record()}).first->second;
        const Time        _creationTime = now(); // Assign last so no lookup overhead
    };

}

// static PerfUtils::StatsScope::Data data;
// PerfUtils::StatsScope td1(data, __PRETTY_FUNCTION__, 1000);
