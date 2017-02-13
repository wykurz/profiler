#include <Profiler/Algorithm/FreeMap.h>
#include <Profiler/Exception/Exception.h>
#include <boost/test/unit_test.hpp>

namespace Profiler { namespace Algorithm { namespace Test
{

    BOOST_AUTO_TEST_SUITE(FreeMapUnitTests)

    BOOST_AUTO_TEST_CASE(OnePass)
    {
        FreeMap bits(FreeMap::MaxSize);
        for (int i = 0; i < FreeMap::MaxSize; ++i)
            BOOST_CHECK_EQUAL(true, bits.isFree(i));
        for (int acquired = 0; acquired < FreeMap::MaxSize; ++acquired) {
            auto index = bits.getFree();
            BOOST_REQUIRE_NE(index, -1);
            BOOST_CHECK_EQUAL(false, bits.isFree(index));
        }
        BOOST_CHECK_EQUAL(-1, bits.getFree());
        for (int i = 0; i < FreeMap::MaxSize; ++i) {
            bits.setFree(i);
            BOOST_CHECK_EQUAL(true, bits.isFree(i));
        }
    }

    BOOST_AUTO_TEST_CASE(SkipOne)
    {
        FreeMap bits(FreeMap::MaxSize);
        for (int i = 0; i < FreeMap::MaxSize; ++i) {
            auto index = bits.getFree();
            BOOST_REQUIRE_NE(index, -1);
            BOOST_CHECK_EQUAL(false, bits.isFree(index));
        }
        int free = 0;
        for (int i = 0; i < FreeMap::MaxSize; i += 2) {
            bits.setFree(i);
            BOOST_CHECK_EQUAL(true, bits.isFree(i));
            ++free;
        }
        for (int acquired = 0; acquired < free; ++acquired) {
            auto index = bits.getFree();
            BOOST_REQUIRE_NE(index, -1);
            BOOST_CHECK_EQUAL(false, bits.isFree(index));
        }
        BOOST_CHECK_EQUAL(-1, bits.getFree());
        for (int i = 0; i < FreeMap::MaxSize; i += 2) {
            bits.setFree(i);
            BOOST_CHECK_EQUAL(true, bits.isFree(i));
        }
    }

    BOOST_AUTO_TEST_CASE(SkipTwelve)
    {
        FreeMap bits(FreeMap::MaxSize);
        for (int i = 0; i < FreeMap::MaxSize; ++i) {
            auto index = bits.getFree();
            BOOST_REQUIRE_NE(index, -1);
            BOOST_CHECK_EQUAL(false, bits.isFree(index));
        }
        int free = 0;
        for (int i = 0; i < FreeMap::MaxSize; i += 13) {
            bits.setFree(i);
            BOOST_CHECK_EQUAL(true, bits.isFree(i));
            ++free;
        }
        for (int acquired = 0; acquired < free; ++acquired) {
            auto index = bits.getFree();
            BOOST_REQUIRE_NE(index, -1);
            BOOST_CHECK_EQUAL(false, bits.isFree(index));
        }
        BOOST_CHECK_EQUAL(-1, bits.getFree());
        for (int i = 0; i < FreeMap::MaxSize; i += 13) {
            bits.setFree(i);
            BOOST_CHECK_EQUAL(true, bits.isFree(i));
        }
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}
}
