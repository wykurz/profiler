#include <Profiler/Algorithm/FreeMap.h>
#include <Profiler/Exception/Exception.h>
#include <boost/test/unit_test.hpp>

namespace Profiler { namespace Algorithm { namespace Test
{

namespace
{

    struct Fixture
    {
        Fixture()
        {
            std::srand(0);
            for (int i = 0; i < test.size(); ++i) bits.getFree();
        }
        void setupTest()
        {
            int free = 0;
            for (int i = 0; i < test.size(); ++i) {
                if (test[i]) {
                    test[i] = false;
                    ++free;
                }
            }
            for (int i = 0; i < free; ++i) bits.getFree();
            for (int i = 0; i < test.size(); ++i) {
                bool flip = std::rand() & 1;
                if (flip) {
                    test[i] = true;
                    bits.setFree(i);
                }
            }
        }
        bool isFree(std::size_t index_)
        {
            return test[index_];
        }
        std::vector<bool> test = std::vector<bool>(1024 + 3, false);
        FreeMap bits{test.size()};
    };

}

    BOOST_FIXTURE_TEST_SUITE(FreeMapStressTests, Fixture)

    BOOST_AUTO_TEST_CASE(Patterns)
    {
        for (int i = 0; i < 1024; ++i) {
            setupTest();
            for (int i = 0; i < test.size(); ++i) BOOST_CHECK_EQUAL(isFree(i), bits.isFree(i));
        }
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}
}
