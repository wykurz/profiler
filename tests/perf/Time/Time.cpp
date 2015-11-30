#include <Time/Time.h>
#include <benchmark/benchmark.h>

static void escape(void* p)
{
    asm volatile("" : : "g"(p) : "memory");
}

template <typename TimeType>
void timeTest(std::size_t createCount_, std::size_t queryCount_)
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

struct TimeBenchFixture: public ::benchmark::Fixture
{
    void SetUp()
    {
    }

    void highResTimeTest(benchmark::State& state_)
    {
        while (state_.KeepRunning())
        {
            timeTest<Time::HighResTime>(state_.range_x(), state_.range_y());
        }
    }

    void systemClockTimeTest(benchmark::State& state_)
    {
        while (state_.KeepRunning())
        {
            timeTest<Time::Time<std::chrono::system_clock>>(state_.range_x(), state_.range_y());
        }
    }

  private:
};

BENCHMARK_DEFINE_F(TimeBenchFixture,     HighResTimeTest)(benchmark::State& state_) {     highResTimeTest(state_); }
BENCHMARK_DEFINE_F(TimeBenchFixture, SystemClockTimeTest)(benchmark::State& state_) { systemClockTimeTest(state_); }
BENCHMARK_REGISTER_F(TimeBenchFixture,     HighResTimeTest)->RangePair(1, 1, 0, 1 << 10);
BENCHMARK_REGISTER_F(TimeBenchFixture, SystemClockTimeTest)->RangePair(1, 1, 0, 1 << 10);

BENCHMARK_MAIN();
