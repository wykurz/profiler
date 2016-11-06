#include <Algorithms/BitMask.h>
#include <Exception/Exception.h>
#include <boost/test/unit_test.hpp>

// TODO: Wrap everything in Profiler namespace
namespace Profiler { namespace Algorithms { namespace Test
{

    BOOST_AUTO_TEST_SUITE(BitMaskTests)

    BOOST_AUTO_TEST_CASE(OneByte)
    {
        FreeMap bits(8);
        BOOST_CHECK_EQUAL(0, bits.firstFree());
        BOOST_CHECK_EQUAL(7, bits.lastFree());
        BOOST_CHECK_EQUAL(true, bits.isFree(0));
        bits.set(0, false);
        BOOST_CHECK_EQUAL(1, bits.firstFree());
        BOOST_CHECK_EQUAL(7, bits.lastFree());
        BOOST_CHECK_EQUAL(false, bits.isFree(0));
        bits.set(1, false);
        BOOST_CHECK_EQUAL(2, bits.firstFree());
        BOOST_CHECK_EQUAL(7, bits.lastFree());
    }

    BOOST_AUTO_TEST_CASE(TwoBytes1)
    {
        FreeMap bits(9);
        BOOST_CHECK_EQUAL(0, bits.firstFree());
        BOOST_CHECK_EQUAL(8, bits.lastFree());
        BOOST_CHECK_EQUAL(true, bits.isFree(0));
        bits.set(0, false);
        BOOST_CHECK_EQUAL(1, bits.firstFree());
        BOOST_CHECK_EQUAL(8, bits.lastFree());
        BOOST_CHECK_EQUAL(false, bits.isFree(0));
        bits.set(1, false);
        BOOST_CHECK_EQUAL(2, bits.firstFree());
        BOOST_CHECK_EQUAL(8, bits.lastFree());
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}
}
