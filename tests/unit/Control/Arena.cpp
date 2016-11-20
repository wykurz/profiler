#include <Control/Arena.h>
#include <set>
#include <vector>
#include <boost/test/unit_test.hpp>

namespace Profiler { namespace Control { namespace Test
{

    BOOST_AUTO_TEST_SUITE(ArenaTests)

    BOOST_AUTO_TEST_CASE(ZeroSize)
    {
        Arena arena(0);
        BOOST_CHECK(nullptr == arena.basePtr<char>());
        BOOST_CHECK(nullptr == arena.acquire<char>());
    }

    BOOST_AUTO_TEST_CASE(SizeOne)
    {
        Arena arena(sizeof(Arena::BlockHolder) * 1);
        auto base = arena.basePtr<int>();
        BOOST_REQUIRE(nullptr != base);
        auto arrPtr = arena.acquire<int>();
        BOOST_REQUIRE(nullptr != arrPtr);
        BOOST_CHECK(base <= &(*arrPtr)[0]);
        BOOST_CHECK(nullptr == arena.acquire<int>());
        arena.release(arrPtr);
        arrPtr = arena.acquire<int>();
        BOOST_REQUIRE(nullptr != arrPtr);
        BOOST_CHECK(base <= &(*arrPtr)[0]);
    }

    BOOST_AUTO_TEST_CASE(Small)
    {
        std::set<std::uintptr_t> dirty;
        auto verifyAcquire = [&dirty](void* ptr_) {
            // Verify that the acquire address doens't conflict with what was previously acquired.
            auto iptr = reinterpret_cast<std::intptr_t>(ptr_);
            if (dirty.find(iptr) != dirty.end()) return 1;
            dirty.insert(iptr);
            auto next = dirty.upper_bound(iptr);
            if (next != dirty.end() && *next < iptr + sizeof(Arena::BlockHolder)) return 2;
            if (iptr < sizeof(Arena::BlockHolder)) return 0;
            auto prev = dirty.lower_bound(iptr - sizeof(Arena::BlockHolder));
            if (*prev != iptr && iptr < *prev + sizeof(Arena::BlockHolder)) return 3;
            return 0;
        };
        auto verifyRelease = [&dirty](void* ptr_) {
            // Verify that releas is of a valid address
            auto iptr = reinterpret_cast<std::intptr_t>(ptr_);
            if (0 == dirty.erase(iptr)) return 1;
            return 0;
        };
        Arena arena(sizeof(Arena::BlockHolder) * 4);
        std::vector<Arena::Block<int>*> ptrs;
        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[0]));
        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[1]));
        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[2]));
        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[3]));

        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE(nullptr == ptrs[4]);
        ptrs.pop_back();

        arena.release(ptrs[3]);
        BOOST_REQUIRE_EQUAL(0, verifyRelease(ptrs[3]));
        ptrs.pop_back();
        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[3]));

        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE(nullptr == ptrs[4]);
        ptrs.pop_back();

        arena.release(ptrs[3]);
        BOOST_REQUIRE_EQUAL(0, verifyRelease(ptrs[3]));
        ptrs.pop_back();
        arena.release(ptrs[2]);
        BOOST_REQUIRE_EQUAL(0, verifyRelease(ptrs[2]));
        ptrs.pop_back();
        arena.release(ptrs[1]);
        BOOST_REQUIRE_EQUAL(0, verifyRelease(ptrs[1]));
        ptrs.pop_back();
        arena.release(ptrs[0]);
        BOOST_REQUIRE_EQUAL(0, verifyRelease(ptrs[0]));
        ptrs.pop_back();

        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[0]));
        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[1]));
        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[2]));
        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE_EQUAL(0, verifyAcquire(ptrs[3]));

        ptrs.push_back(arena.acquire<int>());
        BOOST_REQUIRE(nullptr == ptrs[4]);
        ptrs.pop_back();
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}
}
