#include <Control/Arena.h>
#include <benchmark/benchmark.h>

using namespace Profiler::Control;
struct ArenaBenchFixture: public ::benchmark::Fixture
{
    void acquireTest(benchmark::State& state_)
    {
        const std::size_t nblocks = state_.range(0);
        Arena arena(sizeof(Arena::BlockHolder) * nblocks);
        // Fill up the arena
        std::vector<Arena::Block<int>*> ptrs;
        for (int i = 0; i < nblocks; ++i) ptrs.push_back(arena.acquire<int>());
        auto randRelease = [&arena, &ptrs](int fillRate_) {
            // Randomly drop (1 - fillRate_) % of elements
            for (auto ptr : ptrs) {
                if (std::rand() % 100 < (100 - fillRate_)) arena.release(ptr);
            }
        };
        long acquires = 0;
        std::srand(0);
        while (state_.KeepRunning()) {
            state_.PauseTiming();
            randRelease(state_.range(1));
            state_.ResumeTiming();
            auto ptr = arena.acquire<int>();
            while (ptr) {
                benchmark::DoNotOptimize(ptr = arena.acquire<int>());
                benchmark::ClobberMemory();
                ++acquires;
            }
        }
        state_.SetItemsProcessed(acquires);
    }
};

BENCHMARK_DEFINE_F(ArenaBenchFixture, ArenaTest)(benchmark::State& state_) { acquireTest(state_); }
BENCHMARK_REGISTER_F(ArenaBenchFixture, ArenaTest)->Ranges({{1000, 100000}, {0, 90}});
