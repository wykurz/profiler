#include <Instrumentation/StatsScope.h>
#include <boost/test/unit_test.hpp>

#include <Queue/Queue.h>

namespace Queue { namespace Tests
{

    BOOST_AUTO_TEST_CASE(Basic)
    {
        std::size_t n = 1000;
        std::vector<Node<int>> data;
        for (int i = 0; i < n; ++i) data.push_back(std::move(i));
        Queue<int> queue;
        for (int i = 0; i < n; ++i) queue.push(&data[i]);
        std::vector<Node<int>> data2;
        for (int i = 0; i < n; ++i) {
            auto iptr = queue.pull();
            BOOST_REQUIRE(iptr);
            data2.push_back(*iptr);
        }
        for (int i = 0; i < n; ++i) {
            BOOST_CHECK_EQUAL(data[i].value, data2[n - i - 1].value);
        }
    }

}
}
