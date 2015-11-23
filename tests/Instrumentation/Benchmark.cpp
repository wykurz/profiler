#include <benchmark/benchmark.h>

struct StatScopeFixture: public ::benchmark::Fixture
{
    void SetUp()
    {
    }
};

BENCHMARK_DEFINE_F(StatScopeFixture, SimpleTest)(benchmark::State& state_) {  }
BENCHMARK_REGISTER_F(StatScopeFixture, SimpleTest)->Range(1 << 20, 1 << 20); \

BENCHMARK_MAIN();
