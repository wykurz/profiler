#ifndef _PROFILER_CONTROL_WRITER_H
#define _PROFILER_CONTROL_WRITER_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Control/Allocation.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Log.h>
#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace Profiler {
namespace Writer {

template <typename ConfigType_> struct Processor {
  using ConfigType = ConfigType_;
  using RecordList = typename ConfigType::RecordList;
  // TODO(mateusz): specify sleepTime in the Config
  Processor(ConfigType &config_, Control::HolderArray<RecordList> &holderArray_)
      : _config(config_), _holderArray(holderArray_) {
    DLOG("Created new Processor");
  }
  Processor(const Processor &) = delete;
  ~Processor() { PROFILER_ASSERT(_done.load(std::memory_order_acquire)); }

  /**
   * Iterates over all record holders and writes all data. No other threads can
   * perform logging at this time as
   * there is no synchronization provided.
   */
  void finalPass() {
    finalizeAll();
    onePass();
  }

  /**
   * Puts worker in a loop periodically checking if any thread produced output
   * that needs to be written to disk.
   * After iterating through all threads, worker will sleep for a fixed amount
   * of time.
   */
  void run() {
    auto doRun = [this]() {
      onePass();
      std::this_thread::sleep_for(_config.writerSleepTime);
    };
    do
      doRun();
    while (!_done.load(std::memory_order_acquire));
    // One final run to capture any events that may have been missed due to
    // notification timing
    doRun();
    // for (auto &holder : _holderArray)
    //   holder.flush();
  }

  /**
   * The run loop will eventually terminate after stop() was called.
   */
  void stop() {
    DLOG("Stopping processor!");
    _done.store(true, std::memory_order_release);
  }

  void onePass() {
    Mpl::apply(
        [this](auto &writer_) {
          HolderRecordIter<decltype(writer_)> iter(writer_);
          this->_holderArray.applyAll(iter);
        },
        _config.writers);
  }

private:
  template <typename RecordWriterType_> struct HolderRecordIter {
    explicit HolderRecordIter(RecordWriterType_ &writer_) : _writer(writer_) {}
    template <typename HolderType_> void operator()(HolderType_ &holder_) {
      auto recordIter = holder_.getDirtyRecords();
      auto recordPtr = recordIter.next();
      while (recordPtr) {
        _writer(*recordPtr, holder_.getId());
        recordPtr = recordIter.next();
      }
    }

  private:
    RecordWriterType_ &_writer;
  };
  void finalizeAll() {
    _holderArray.applyAll([](auto &holder_) { holder_.finalize(); });
  }
  ConfigType &_config;
  Control::HolderArray<RecordList> &_holderArray;
  std::atomic<bool> _done{false};
};

} // namespace Writer
} // namespace Profiler

#endif
