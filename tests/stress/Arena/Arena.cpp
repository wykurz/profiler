#include <Profiler/Control/Arena.h>
#include <unordered_set>
#include <vector>
#include <boost/test/unit_test.hpp>

namespace Profiler { namespace Control { namespace Test
{

    BOOST_AUTO_TEST_SUITE(ArenaTests)

    BOOST_AUTO_TEST_CASE(RandomWalk)
    {
        const std::size_t nblocks = 10000;
        Arena arena(sizeof(Arena::BlockHolder) * nblocks);
        std::unordered_set<std::uintptr_t> dirty;
        auto verifyAcquire = [&dirty](void* ptr_) {
            auto iptr = reinterpret_cast<std::uintptr_t>(ptr_);
            if (dirty.find(iptr) != dirty.end()) return false;
            dirty.insert(iptr);
            return true;
        };
        std::srand(0);
        for (int loops = 0; loops < 3; ++loops) {
            for (int i = 0; i < nblocks; ++i) BOOST_REQUIRE(verifyAcquire(arena.acquire<int>()));
            BOOST_REQUIRE(dirty.find(reinterpret_cast<std::uintptr_t>(nullptr)) == dirty.end());
            BOOST_REQUIRE(nullptr == arena.acquire<int>());
            {
                std::vector<std::uintptr_t> dvec(dirty.begin(), dirty.end());
                std::sort(dvec.begin(), dvec.end());
                for (int i = 0; i < nblocks - 1; ++i) BOOST_CHECK(sizeof(Arena::BlockHolder) <= dvec[i + 1] - dvec[i]);
            }
            auto randomErase = [&arena, &dirty]() {
                for (auto it = dirty.begin(); it != dirty.end();) {
                    bool remove = std::rand() & 1;
                    if (remove) {
                        arena.release(reinterpret_cast<Arena::Block<int>*>(*it));
                        it = dirty.erase(it);
                    }
                    else ++it;
                }
            };
            while (0 < dirty.size()) {
                randomErase();
                for (int i = 0; i < 10; ++i) BOOST_REQUIRE(verifyAcquire(arena.acquire<int>()));
                auto it = dirty.begin();
                for (int i = 0; i < 10; ++i) {
                    arena.release(reinterpret_cast<Arena::Block<int>*>(*it));
                    it = dirty.erase(it);
                }
            }
        }
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}
}
