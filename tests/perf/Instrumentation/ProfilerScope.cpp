#include <Profiler/Instrumentation/ProfilerScope.h>
#include <benchmark/benchmark.h>

namespace Profiler {
namespace Instrumentation {
namespace Test {

void statsScopeTest(benchmark::State &state_) {
  Profiler::Control::getManager().stopWriter();
  const int queryCount_ = state_.range(0);
  long processed = 0;
  while (state_.KeepRunning()) {
    for (int i = 0; i < queryCount_; ++i)
      PROFILER_SCOPE();
    state_.PauseTiming();
    processed += queryCount_;
    Profiler::Control::getManager().writerFinalPass();
    state_.ResumeTiming();
  }
  state_.SetItemsProcessed(processed);
}

struct ProfilerScopeFixture : public ::benchmark::Fixture {};

BENCHMARK_DEFINE_F(ProfilerScopeFixture, ProfilerScopeTest)
(benchmark::State &state_) { statsScopeTest(state_); }
BENCHMARK_REGISTER_F(ProfilerScopeFixture, ProfilerScopeTest)
    ->Range(1, 1 << 20);
} // namespace Test
} // namespace Instrumentation
} // namespace Profiler
