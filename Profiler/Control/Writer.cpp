#include <Profiler/Control/Writer.h>
#include <Profiler/Exception/Exception.h>
#include <chrono>
#include <thread>
#include <utility>

namespace Profiler {
namespace Control {

Writer::Writer(HolderArray &threadArray_, std::chrono::microseconds sleepTime_)
    : _threadArray(threadArray_), _sleepTime(sleepTime_) {}

Writer::~Writer() { PROFILER_ASSERT(_done.load(std::memory_order_acquire)); }

void Writer::finalPass() {
  for (auto &holder : this->_threadArray) {
    auto lk = holder.lock();
    holder.finalize();
    holder.streamDirtyRecords();
    holder.flush();
  }
}

void Writer::run() {
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

void Writer::stop() { _done.store(true, std::memory_order_release); }

void Writer::onePass() {
  for (auto &holder : this->_threadArray) {
    auto lk = holder.lock();
    holder.streamDirtyRecords();
  }
}
}
}
