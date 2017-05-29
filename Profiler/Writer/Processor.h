#ifndef _PROFILER_CONTROL_WRITER_H
#define _PROFILER_CONTROL_WRITER_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Control/Allocation.h>
#include <Profiler/Control/Holder.h>
#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace Profiler {
namespace Writer { // TODO(mateusz): Move to Writer namespace?

struct WriteToFile {
  template <typename RecortType_>
  void operator()(const RecortType_ &record_) {
    std::cerr << "Saw record type " << typeid(record_).name() << "\n";
  }
};

template <typename ConfigType_>
struct Processor {
  using ConfigType = ConfigType_;
  using RecordList = typename ConfigType::RecordList;
  // TODO(mateusz): specify sleepTime in the Config
  Processor(const ConfigType& config_, Control::HolderArray<RecordList> &holderArray_)
      : _config(config_), _holderArray(holderArray_) { }
  Processor(const Processor &) = delete;
  ~Processor() { PROFILER_ASSERT(_done.load(std::memory_order_acquire)); }

  /**
   * Iterates over all record holders and writes all data. No other threads can
   * perform logging at this time as
   * there is no synchronization provided.
   */
  void finalPass() {
    // for (auto &holder : this->_holderArray) {
    //   auto lk = holder.lock();
    //   holder.finalize();
    //   holder.streamDirtyRecords();
    //   holder.flush();
    // }
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
  void stop() { _done.store(true, std::memory_order_release); }

private:
  void onePass() {
    Mpl::apply<typename ConfigType::WriterList>([this](auto dummy_) {
        using WriterType = typename decltype(dummy_)::Type;
        this->_holderArray.applyAll(WriterType());
      });
  }

  const ConfigType& _config;
  Control::HolderArray<RecordList> &_holderArray;
  std::atomic<bool> _done{false};
};

// struct OutputFactory {
//   virtual ~OutputFactory() = default;
//   virtual Output::Ptr newOutput(std::size_t extractorId_) const = 0;
// };

// namespace Internal {

// struct FileOut : Output {
//   explicit FileOut(const std::string &name_)
//       : _out(name_, std::fstream::binary | std::fstream::trunc) {
//     DLOG("FileOut " << name_ << " " << std::size_t(&_out));
//   }
//   std::ostream &get() override { return _out; }
//   void flush() override { _out.flush(); }

// private:
//   std::ofstream _out;
// };
// } // namespace Internal

// struct FileOutputs : OutputFactory {
//   explicit FileOutputs(const Config &config_) : _config(config_) {}
//   Output::Ptr newOutput(std::size_t extractorId_) const override {
//     return std::make_unique<Internal::FileOut>(_config.binaryLogPrefix + "." +
//                                                std::to_string(extractorId_));
//   }

// private:
//   const Config &_config;
// };

} // namespace Control
} // namespace Profiler

#endif
