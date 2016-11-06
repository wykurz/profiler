#include <Algorithms/FreeMap.h>
#include <Exception/Exception.h>
#include <boost/test/unit_test.hpp>

namespace Profiler { namespace Algorithms { namespace Test
{

namespace
{

    struct Fixture
    {
        Fixture()
        {
            std::srand(0);
        }
        void setupTest()
        {
            for (int i = 0; i < test.size(); ++i) {
                bool flip = std::rand() & 1;
                if (flip) {
                    test[i] = !test[i];
                    bits.set(i, test[i]);
                }
            }
        }
        int firstFree() const
        {
            for (int i = 0; i < test.size(); ++i) if (test[i]) return i;
            return -1;
        }
        int lastFree() const
        {
            for (int i = test.size() - 1; 0 <= i; --i) if (test[i]) return i;
            return -1;
        }
        bool isFree(std::size_t index_)
        {
            return test[index_];
        }
        std::vector<bool> test = std::vector<bool>(1024 + 3, true);
        FreeMap bits{test.size()};
    };

}

    BOOST_FIXTURE_TEST_SUITE(FreeMapStressTests, Fixture)

    BOOST_AUTO_TEST_CASE(Patterns)
    {
        for (int i = 0; i < 1024; ++i) {
            setupTest();
            BOOST_CHECK_EQUAL(firstFree(), bits.firstFree());
            BOOST_CHECK_EQUAL(lastFree(), bits.lastFree());
            for (int i = 0; i < test.size(); ++i) BOOST_CHECK_EQUAL(isFree(i), bits.isFree(i));
        }
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}
}
