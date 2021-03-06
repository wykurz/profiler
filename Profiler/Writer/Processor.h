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

template <typename ConfigType_, typename... Writers_> struct Processor {
  using ConfigType = ConfigType_;
  using RecordList = typename ConfigType::RecordList;
  // TODO(mateusz): specify sleepTime in the Config
  Processor(Control::HolderArray<RecordList> &holderArray_, ConfigType &config_,
            Writers_ &&... writers_)
      : _holderArray(holderArray_), _config(config_),
        _writers(std::forward<Writers_>(writers_)...) {
    DLOG("Created new Processor");
  }
  Processor(const Processor &) = delete;
  ~Processor() { PROFILER_ASSERT(_done.load(std::memory_order_acquire)); }

  void finalPass() {
    if (_finished)
      return;
    finalizeAll();
    onePass();
    Mpl::apply([this](auto &writer_) { writer_.finish(); }, _writers);
    _finished = true;
  }

  void run() {
    do {
      onePass();
      std::this_thread::sleep_for(_config.writerSleepTime);
    } while (!_done.load(std::memory_order_acquire));
    // One final run to capture any events that may have been missed
    onePass();
  }

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
        _writers);
  }

private:
  template <typename RecordWriterType_> struct HolderRecordIter {
    explicit HolderRecordIter(RecordWriterType_ &writer_) : _writer(writer_) {}
    template <typename HolderType_> void operator()(HolderType_ &holder_) {
      auto recordIter = holder_.getDirtyRecords();
      auto recordPtr = recordIter.next();
      while (recordPtr) {
        _writer(*recordPtr, holder_.getId(), holder_.getUserContext());
        recordPtr = recordIter.next();
      }
    }

  private:
    RecordWriterType_ &_writer;
  };
  void finalizeAll() {
    _holderArray.applyAll([](auto &holder_) { holder_.finalize(); });
  }
  Control::HolderArray<RecordList> &_holderArray;
  ConfigType &_config;
  std::tuple<Writers_...> _writers;
  std::atomic<bool> _done{false};
  bool _finished = false;
};

} // namespace Writer
} // namespace Profiler

#endif
