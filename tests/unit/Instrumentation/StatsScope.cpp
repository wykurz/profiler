#include <Control/Manager.h>
#include <Instrumentation/StatsScope.h>
#include <boost/test/unit_test.hpp>

namespace Instrumentation { namespace Test {

namespace {

    int testFunc2()
    {
        STATS_SCOPE();
        int mod = 107;
        int res = 1;
        for (int i = 2; i < 1e7; ++i) res *= 2, res %= mod;
        return res;
    }

    void testFunc1()
    {
        STATS_SCOPE();
        testFunc2();
        testFunc2();
    }

}

    BOOST_AUTO_TEST_CASE(Basic)
    {
        testFunc1();
        BOOST_CHECK_EQUAL(std::size_t(3), Control::getThread().getRecordManager<Record::Record>().dirty.size());
    }

}
}
