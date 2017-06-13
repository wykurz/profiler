#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Instrumentation.h>
#include <benchmark/benchmark.h>

namespace Profiler {
namespace Instrumentation {
namespace Test {

namespace {

struct NoopWriter {
  template <typename RecortType_>
  void operator()(const RecortType_ & /*unused*/,
                  std::size_t /*unused*/) const {}
};

struct TestGlobals {
  static Control::Manager &getManager() { return *manager(); }
  template <typename StorageType_>
  static Control::ThreadRecords<StorageType_> &getThreadRecords() {
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

void statsScopeTest(benchmark::State &state_) {
  using RecordType = Record::ScopeRecord<Clock::Rdtsc>;
  using StorageType = typename RecordType::Storage;
  using TestConfig = Config<Mpl::TypeList<StorageType>, NoopWriter>;
  TestConfig config{NoopWriter()};
  Control::ManagerImpl<TestConfig> manager(config);
  TestGlobals::setManagerPtr(&manager);
  Control::ThreadRecords<StorageType> threadRecords(
      manager.addThreadRecords<StorageType>());
  TestGlobals::setThreadRecordsPtr(&threadRecords);
  const int queryCount_ = state_.range(0);
  long processed = 0;
  while (state_.KeepRunning()) {
    for (int i = 0; i < queryCount_; ++i)
      Instrumentation::ProfilerScope<typename StorageType::Clock, TestGlobals>(
          "foobar");
    state_.PauseTiming();
    processed += queryCount_;
    manager.processorFinalPass();
    state_.ResumeTiming();
  }
  state_.SetItemsProcessed(processed);
}

struct ProfilerScopeFixture : public ::benchmark::Fixture {};

BENCHMARK_DEFINE_F(ProfilerScopeFixture, ProfilerScopeTest)
(benchmark::State &state_) { statsScopeTest(state_); }
BENCHMARK_REGISTER_F(ProfilerScopeFixture, ProfilerScopeTest)
    ->Range(1, 1 << 20);
} // namespace Test
} // namespace Instrumentation
} // namespace Profiler
