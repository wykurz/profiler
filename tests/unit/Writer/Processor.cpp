#include <Profiler/Clock.h>
#include <Profiler/Control/Allocation.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Instrumentation.h>
#include <Profiler/Writer/Processor.h>
#include <boost/test/unit_test.hpp>
#include <thread>
#include <unordered_map>

namespace Profiler {
namespace Writer {
namespace Test {

namespace {

using BufferMap = std::unordered_map<std::string, std::string>;

struct MemoryWriter {
  template <typename RecortType_>
  void operator()(const RecortType_ &record_,
                  const std::string & /*userContext_*/) {
    const std::type_index type = typeid(record_);
    ++seenCount[type];
  }
  // TODO(mateusz): Make non-static once we can pass actual function objects
  using SeenMap = std::unordered_map<std::type_index, std::size_t>;
  static SeenMap seenCount;
};

MemoryWriter::SeenMap MemoryWriter::seenCount;

struct TestGlobals {
  static Control::Manager &getManager() {
    BOOST_REQUIRE(manager());
    return *manager();
  }
  template <typename StorageType_>
  static Control::ThreadRecords<StorageType_> &getThreadRecords() {
    BOOST_REQUIRE(threadRecords<StorageType_>());
    return *threadRecords<StorageType_>();
  }
  static void setManagerPtr(Control::Manager *manager_) {
    manager() = manager_;
  }
  template <typename StorageType_>
  static void
  setThreadRecordsPtr(Control::ThreadRecords<StorageType_> *threadRecords_) {
    threadRecords<StorageType_>() = threadRecords_;
  }

private:
  static Control::Manager *&manager() {
    static Control::Manager *_manager = nullptr;
    return _manager;
  }
  template <typename StorageType>
  static Control::ThreadRecords<StorageType> *&threadRecords() {
    static Control::ThreadRecords<StorageType> *_threadRecords = nullptr;
    return _threadRecords;
  }
};
} // namespace

BOOST_AUTO_TEST_SUITE(WriterTests)

BOOST_AUTO_TEST_CASE(Basic) {
  using R1 = Record::ScopeRecord<Clock::Rdtsc>;
  using R2 = Record::ScopeRecord<Clock::Steady>;
  using R3 = Record::ScopeRecord<Clock::System>;
  using RecordTypeList = Mpl::TypeList<R1::Storage, R2::Storage, R3::Storage>;
  using TestConfig = Config<RecordTypeList, MemoryWriter>;
  TestConfig config{MemoryWriter()};
  Control::ManagerImpl<TestConfig> manager(config);
  TestGlobals::setManagerPtr(&manager);
  auto addRecords = [&](auto recordType_, auto numAdd_) {
    using RecordType = typename decltype(recordType_)::Type;
    using StorageType = typename RecordType::Storage;
    Control::ThreadRecords<StorageType> threadRecords(
        manager.addThreadRecords<StorageType>());
    TestGlobals::setThreadRecordsPtr(&threadRecords);
    auto oneRecord = [&threadRecords]() {
      Instrumentation::ProfilerScope<typename StorageType::Clock, TestGlobals>(
          "foobar");
    };
    for (int i = 0; i < numAdd_; ++i)
      oneRecord();
  };
  addRecords(Mpl::TypeInfo<R1>(), 3);
  addRecords(Mpl::TypeInfo<R2>(), 1);
  addRecords(Mpl::TypeInfo<R3>(), 32 * 1024);
  manager.processorFinalPass();
  manager.stopProcessor();
  BOOST_CHECK_EQUAL(MemoryWriter::seenCount[typeid(R1::Storage)], 3);
  BOOST_CHECK_EQUAL(MemoryWriter::seenCount[typeid(R2::Storage)], 1);
  BOOST_CHECK_EQUAL(MemoryWriter::seenCount[typeid(R3::Storage)], 32 * 1024);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace Test
} // namespace Writer
} // namespace Profiler
