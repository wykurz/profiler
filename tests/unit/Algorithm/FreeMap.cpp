#include <Profiler/Algorithm/FreeMap.h>
#include <Profiler/Exception/Exception.h>
#include <boost/test/unit_test.hpp>

namespace Profiler { namespace Algorithm { namespace Test
{

    BOOST_AUTO_TEST_SUITE(FreeMapUnitTests)

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

    BOOST_AUTO_TEST_CASE(TwoBytes)
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

    BOOST_AUTO_TEST_CASE(EightBytes)
    {
        FreeMap bits(8 * 8 + 1);
        BOOST_CHECK_EQUAL(0, bits.firstFree());
        BOOST_CHECK_EQUAL(8 * 8, bits.lastFree());
        bits.set(0, false);
        BOOST_CHECK_EQUAL(1, bits.firstFree());
        BOOST_CHECK_EQUAL(8 * 8, bits.lastFree());
        BOOST_CHECK_EQUAL(false, bits.isFree(0));
        bits.set(8 * 8, false);
        BOOST_CHECK_EQUAL(1, bits.firstFree());
        BOOST_CHECK_EQUAL(8 * 8 - 1, bits.lastFree());
        // Reverse
        bits.set(8 * 8, true);
        BOOST_CHECK_EQUAL(1, bits.firstFree());
        BOOST_CHECK_EQUAL(8 * 8, bits.lastFree());
        bits.set(4 * 8, true);
        BOOST_CHECK_EQUAL(1, bits.firstFree());
        BOOST_CHECK_EQUAL(8 * 8, bits.lastFree());
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}
}
