#include <Control/Manager.h>
#include <Control/Thread.h>
#include <Instrumentation/StatsScope.h>
#include <boost/test/unit_test.hpp>

namespace Profiler { namespace Instrumentation { namespace Test
{

namespace
{

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

    struct Fixture
    {
        Fixture()
        {
            Control::getManager().stopWriter();
        }
    };

}

    BOOST_FIXTURE_TEST_SUITE(StatsScopeTests, Fixture)

    BOOST_AUTO_TEST_CASE(Basic)
    {
        testFunc1();
        auto recordNode = Control::getThread().template getRecordManager<Record::Record>().extractDirtyRecords();
        BOOST_REQUIRE(recordNode);
        int size = 0;
        while (recordNode) {
            recordNode = recordNode->getNext();
            ++size;
        }
        BOOST_CHECK_EQUAL(3, size);
    }

    BOOST_AUTO_TEST_SUITE_END()

}
}
}
