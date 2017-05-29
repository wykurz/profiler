#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Config.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Instrumentation.h>
#include <boost/test/unit_test.hpp>

namespace Profiler {
namespace Instrumentation {
namespace Test {

namespace {

int testFunc2() {
  PROFILER_SCOPE(Rdtsc);
  int mod = 107;
  int res = 1;
  for (int i = 0; i < 10; ++i)
    res *= 2, res %= mod;
  return res;
}

void testFunc1() {
  PROFILER_SCOPE(Steady);
  testFunc2();
  testFunc2();
}

struct Fixture {
  Fixture() {
    BOOST_REQUIRE(!Control::Internal::managerInstancePtr());
  }
  ~Fixture() {
    Control::Internal::managerInstancePtr() = nullptr;
  }
};

struct MemoryWriter {
  template <typename RecortType_>
  void operator()(const RecortType_ &record_) {
    const std::type_index type = typeid(record_);
    ++seenCount[type];
  }
  // TODO: Make non-static once we can pass actual function objects
  using SeenMap = std::unordered_map<std::type_index, std::size_t>;
  static SeenMap seenCount;
};

MemoryWriter::SeenMap MemoryWriter::seenCount;
} // namespace

BOOST_FIXTURE_TEST_SUITE(ProfilerScopeTests, Fixture)

BOOST_AUTO_TEST_CASE(Basic) {
  using R1 = Record::ScopeRecord<Clock::Rdtsc>;
  using R2 = Record::ScopeRecord<Clock::Steady>;
  {
    using RecordTypeList = Mpl::TypeList<R1::Storage, R2::Storage>;
    using TestConfig = Config<RecordTypeList, Mpl::TypeList<MemoryWriter> >;
    TestConfig config;
    Control::ManagerImpl<TestConfig> manager{config};
    Control::Internal::managerInstancePtr() = &manager;
    BOOST_REQUIRE(Control::Internal::managerInstancePtr());
    manager.startProcessor();
    testFunc1();
  }
  BOOST_CHECK_EQUAL(MemoryWriter::seenCount[typeid(R1::Storage)], 2);
  BOOST_CHECK_EQUAL(MemoryWriter::seenCount[typeid(R2::Storage)], 1);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace Test
} // namespace Instrumentation
} // namespace Profiler
