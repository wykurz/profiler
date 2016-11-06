#include <Instrumentation/Time.h>
#include <benchmark/benchmark.h>

static void escape(void* p)
{
    asm volatile("" : : "g"(p) : "memory");
}

template <typename TimeType>
void timeTest(int createCount_, int queryCount_)
{
    for (int i = 0; i < createCount_; ++i)
    {
        TimeType time;
        for (int j = 0; j < queryCount_; ++j)
        {
            auto delta = time.delta();
            escape(reinterpret_cast<void*>(&delta));
        }
    }
}

using namespace Profiler::Time;
struct TimeBenchFixture: public ::benchmark::Fixture
{
    void highResTimeTest(benchmark::State& state_)
    {
        while (state_.KeepRunning())
        {
            timeTest<HighResTime>(state_.range(0), state_.range(1));
        }
    }

    void systemClockTimeTest(benchmark::State& state_)
    {
        while (state_.KeepRunning())
        {
            timeTest<Time<std::chrono::system_clock>>(state_.range(0), state_.range(1));
        }
    }

    void rdtscClockTimeTest(benchmark::State& state_)
    {
        while (state_.KeepRunning())
        {
            timeTest<RdtscTime>(state_.range(0), state_.range(1));
        }
    }
};

BENCHMARK_DEFINE_F(TimeBenchFixture,     HighResTimeTest)(benchmark::State& state_) {     highResTimeTest(state_); }
BENCHMARK_DEFINE_F(TimeBenchFixture, SystemClockTimeTest)(benchmark::State& state_) { systemClockTimeTest(state_); }
BENCHMARK_DEFINE_F(TimeBenchFixture,  RdtscClockTimeTest)(benchmark::State& state_) {  rdtscClockTimeTest(state_); }
BENCHMARK_REGISTER_F(TimeBenchFixture,     HighResTimeTest)->RangePair(1, 1, 0, 1 << 10);
BENCHMARK_REGISTER_F(TimeBenchFixture, SystemClockTimeTest)->RangePair(1, 1, 0, 1 << 10);
BENCHMARK_REGISTER_F(TimeBenchFixture,  RdtscClockTimeTest)->RangePair(1, 1, 0, 1 << 10);

BENCHMARK_MAIN();
