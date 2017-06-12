#include <Profiler/Algorithm/Mpl.h>
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <tuple>

namespace Profiler {
namespace Mpl {
namespace Test {

BOOST_AUTO_TEST_SUITE(MplUnitTests)

BOOST_AUTO_TEST_CASE(ApplyTuple) {
  std::stringstream ss;
  auto tup = std::make_tuple(1, 0.5, "abc");
  apply([&ss](const auto &arg_) { ss << arg_; }, tup);
  BOOST_CHECK_EQUAL(ss.str(), "10.5abc");
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace Test
} // namespace Algorithm
} // namespace Profiler
