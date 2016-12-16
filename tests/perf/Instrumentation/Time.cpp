#include <Profiler/Instrumentation/Time.h>
#include <benchmark/benchmark.h>

template <typename TimeType>
void timeTest(benchmark::State& state_)
{
    const int createCount_ = state_.range(0);
    const int queryCount_ = state_.range(1);
    long processed = 0;
    while (state_.KeepRunning()) {
        for (int i = 0; i < createCount_; ++i) {
            TimeType time;
            for (int j = 0; j < queryCount_; ++j) {
                benchmark::DoNotOptimize(time.delta());
                benchmark::ClobberMemory();
            }
        }
        processed += createCount_ * queryCount_;
    }
    state_.SetItemsProcessed(processed);
}

using namespace Profiler::Time;
struct TimeBenchFixture: public ::benchmark::Fixture { };

BENCHMARK_DEFINE_F(TimeBenchFixture, HighResTimeTest)(benchmark::State& state_) { timeTest<HighResTime>(state_); }
BENCHMARK_DEFINE_F(TimeBenchFixture, SystemClockTimeTest)(benchmark::State& state_) { timeTest<Time<std::chrono::system_clock> >(state_); }
BENCHMARK_DEFINE_F(TimeBenchFixture, RdtscClockTimeTest)(benchmark::State& state_) { timeTest<RdtscTime>(state_); }
BENCHMARK_REGISTER_F(TimeBenchFixture, HighResTimeTest)->RangePair(1, 1, 1, 1 << 10);
BENCHMARK_REGISTER_F(TimeBenchFixture, SystemClockTimeTest)->RangePair(1, 1, 1, 1 << 10);
BENCHMARK_REGISTER_F(TimeBenchFixture, RdtscClockTimeTest)->RangePair(1, 1, 1, 1 << 10);
