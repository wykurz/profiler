#ifndef _PROFILER_CONTROL_WRITER_H
#define _PROFILER_CONTROL_WRITER_H

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
namespace Control {

/**
 * Writer is responsible for collecting data from all the threads and writing it
 * to the output.
 */
struct Writer {
  /**
   * Writer takes output pointer, global thread array and time interval of how
   * long it should sleep between each
   * activity period.
   */
  Writer(HolderArray &threadArray_, std::chrono::microseconds sleepTime_)
      : _threadArray(threadArray_), _sleepTime(std::move(sleepTime_)) {}
  Writer(const Writer &) = delete;
  ~Writer() { PROFILER_ASSERT(_done.load(std::memory_order_acquire)); }

  /**
   * Iterates over all record holders and writes all data. No other threads can
   * perform logging at this time as
   * there is no synchronization provided.
   */
  void finalPass() {
    for (auto &holder : this->_threadArray) {
      auto lk = holder.lock();
      holder.finalize();
      holder.streamDirtyRecords();
      holder.flush();
    }
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
      std::this_thread::sleep_for(_sleepTime);
    };
    do
      doRun();
    while (!_done.load(std::memory_order_acquire));
    // One final run to capture any events that may have been missed due to
    // notification timing
    doRun();
    for (auto &holder : _threadArray)
      holder.flush();
  }

  /**
   * The run loop will eventually terminate after stop() was called.
   */
  void stop() { _done.store(true, std::memory_order_release); }

private:
  void onePass() {
    for (auto &holder : this->_threadArray) {
      auto lk = holder.lock();
      holder.streamDirtyRecords();
    }
  }

  HolderArray &_threadArray;
  const std::chrono::microseconds _sleepTime;
  std::atomic<bool> _done{false};
};
} // namespace Control
} // namespace Profiler

#endif
