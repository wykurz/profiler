#include <Profiler/Queue/Queue.h>
#include <boost/test/unit_test.hpp>

namespace Profiler {
namespace Queue {
namespace Test {

BOOST_AUTO_TEST_SUITE(QueueTests)

BOOST_AUTO_TEST_CASE(PushPull) {
  int n = 10;
  std::vector<Queue<int>::Node> v(n);
  for (int i = 0; i < 10; ++i)
    v[i].value = i;
  Queue<int> q(&v[0]);
  for (auto &e : v)
    q.push(&e);
  BOOST_CHECK_EQUAL(10, q.size());
  for (int i = 9; 0 <= i; --i) {
    auto ni = q.pull();
    BOOST_REQUIRE(ni);
    BOOST_CHECK_EQUAL(i, ni->value);
  }
  BOOST_CHECK(!q.pull());
}

BOOST_AUTO_TEST_CASE(Extract) {
  int n = 10;
  std::vector<Queue<int>::Node> v(n);
  for (int i = 0; i < 10; ++i)
    v[i].value = i;
  Queue<int> q(&v[0]);
  for (auto &e : v)
    q.push(&e);
  BOOST_CHECK_EQUAL(10, q.size());
  auto ni = q.extract();
  BOOST_REQUIRE_EQUAL(0, q.size());
  for (int i = 9; 0 <= i; ni = ni->getNext(), --i) {
    BOOST_REQUIRE(ni);
    BOOST_CHECK_EQUAL(i, ni->value);
  }
  BOOST_REQUIRE(nullptr == q.extract());
  q.push(&(v[0]));
  ni = q.extract();
  BOOST_CHECK_EQUAL(0, ni->value);
}

BOOST_AUTO_TEST_SUITE_END()
}
}
}
