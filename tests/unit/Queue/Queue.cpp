#include <Queue/Queue.h>
#include <boost/test/unit_test.hpp>

namespace Queue { namespace Test {

    BOOST_AUTO_TEST_CASE(PushPull)
    {
        std::vector<Node<int>> v;
        for (int i = 0; i < 10; ++i) v.push_back(i);
        Queue<int> q(&v[0], 10);
        for (auto& e : v) q.push(&e);
        BOOST_CHECK_EQUAL(10, q.size());
        for (int i = 9; 0 <= i; --i)
        {
            auto ni = q.pull();
            BOOST_REQUIRE(ni);
            BOOST_CHECK_EQUAL(i, ni->value);
        }
        BOOST_CHECK(!q.pull());
    }

    BOOST_AUTO_TEST_CASE(Extract)
    {
        std::vector<Node<int>> v;
        for (int i = 0; i < 10; ++i) v.push_back(i);
        Queue<int> q(&v[0], 10);
        for (auto& e : v) q.push(&e);
        BOOST_CHECK_EQUAL(10, q.size());
        auto ni = q.extract();
        for (int i = 9; 0 <= i; ni = ni->next, --i)
        {
            BOOST_REQUIRE(ni);
            BOOST_CHECK_EQUAL(i, ni->value);
        }
    }

}
}
