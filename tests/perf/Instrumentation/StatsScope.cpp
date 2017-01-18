#include <Profiler/Instrumentation/StatsScope.h>
#include <benchmark/benchmark.h>

void statsScopeTest(benchmark::State& state_)
{
    const int queryCount_ = state_.range(0);
    long processed = 0;
    while (state_.KeepRunning()) {
        for (int i = 0; i < queryCount_; ++i) STATS_SCOPE();
        state_.PauseTiming();
        processed += queryCount_;
        Profiler::Control::getManager().writerOnePass();
        state_.ResumeTiming();
    }
    state_.SetItemsProcessed(processed);
}

using namespace Profiler::Time;
struct StatsScopeFixture: public ::benchmark::Fixture { };

BENCHMARK_DEFINE_F(StatsScopeFixture, StatsScopeTest)(benchmark::State& state_) { statsScopeTest(state_); }
BENCHMARK_REGISTER_F(StatsScopeFixture, StatsScopeTest)->Range(1, 1 << 5);
