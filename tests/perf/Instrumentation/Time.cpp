#include <Profiler/Rdtsc.h>
#include <benchmark/benchmark.h>

namespace Profiler {
namespace Instrumentation {
namespace Test {

template <typename Clock_> void timeTest(benchmark::State &state_) {
  const int createCount_ = state_.range(0);
  const int queryCount_ = state_.range(1);
  long processed = 0;
  while (state_.KeepRunning()) {
    for (int i = 0; i < createCount_; ++i) {
      for (int j = 0; j < queryCount_; ++j) {
        benchmark::DoNotOptimize(Clock_::now());
      }
    }
    processed += createCount_ * queryCount_;
  }
  state_.SetItemsProcessed(processed);
}

struct TimeBenchFixture : public ::benchmark::Fixture {};

BENCHMARK_DEFINE_F(TimeBenchFixture, RtdscTimeTest)(benchmark::State &state_) {
  timeTest<Rdtsc>(state_);
}
BENCHMARK_REGISTER_F(TimeBenchFixture, RtdscTimeTest)
    ->RangePair(1, 1, 1, 1 << 10);
} // namespace Test
} // namespace Instrumentation
} // namespace Profiler
