#include <Profiler/Control/Allocation.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Control/ThreadRecords.h>
#include <Profiler/Writer/Processor.h>
#include <Profiler/Instrumentation.h>
#include <Profiler/Clock.h>
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

BOOST_AUTO_TEST_SUITE(WriterTests)

BOOST_AUTO_TEST_CASE(Basic) {
  using R1 = Record::ScopeRecord<Clock::Rdtsc>;
  using R2 = Record::ScopeRecord<Clock::Steady>;
  using R3 = Record::ScopeRecord<Clock::System>;
  using RecordTypeList = Mpl::TypeList<R1::Storage, R2::Storage, R3::Storage>;
  Control::Arena arena{10000000};
  Control::HolderArray<RecordTypeList> holderArray;
  auto addRecords = [&](auto recordType_, auto numAdd_) {
    using RecordType = typename decltype(recordType_)::Type;
    using StorageType = typename RecordType::Storage;
    auto holderPtr = static_cast<Control::Holder<StorageType>*>(holderArray.findHolder(typeid(StorageType)));
    BOOST_REQUIRE(holderPtr);
    Control::ThreadRecords<StorageType> threadRecords(Control::Allocation<StorageType>(0, arena, *holderPtr));
    auto oneRecord = [&threadRecords]() {
      RecordType record("test");
      Instrumentation::Internal::doRecord(threadRecords.getRecordManager(), record.finish());
    };
    for (int i = 0; i < numAdd_; ++i) oneRecord();
  };
  addRecords(Mpl::TypeInfo<R1>(), 3);
  addRecords(Mpl::TypeInfo<R2>(), 1);
  addRecords(Mpl::TypeInfo<R3>(), 32 * 1024);
  using TestConfig = Config<RecordTypeList, Mpl::TypeList<MemoryWriter> >;
  TestConfig config;
  Writer::Processor<TestConfig> processor{config, holderArray};
  processor.finalPass();
  processor.stop();
  BOOST_CHECK_EQUAL(MemoryWriter::seenCount[typeid(R1::Storage)], 3);
  BOOST_CHECK_EQUAL(MemoryWriter::seenCount[typeid(R2::Storage)], 1);
  BOOST_CHECK_EQUAL(MemoryWriter::seenCount[typeid(R3::Storage)], 32 * 1024);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace Test
} // namespace Control
} // namespace Profiler
