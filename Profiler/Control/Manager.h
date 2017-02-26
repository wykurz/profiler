#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Profiler/Config/Config.h>
#include <Profiler/Control/Allocation.h>
#include <Profiler/Control/Arena.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Control/Writer.h>
#include <atomic>
#include <thread>

namespace Profiler {
namespace Control {

struct Manager {
  static constexpr std::size_t MaxThreads = 1024;

  Manager(const Config::Config &config_, bool startWriter_ = true);
  Manager(const Manager &) = delete;
  ~Manager();

  template <typename Record_> Allocation addThreadRecords() {
    int count = MaxThreads;
    while (0 < count--) {
      std::size_t id = _currentThread++;
      // TODO: Add stress tests with tons of threads...
      auto &holder = _threadArray[id % _threadArray.size()];
      auto lk = holder.lock();
      if (!holder.isEmpty())
        continue;
      auto out = _fileOutputs.newOutput(id);
      Decoder::setupStream<Record_>(out->get());
      holder.setOut(std::move(out));
      return {std::move(lk), _arena, holder};
    }
    ++_droppedThreads;
    return {};
  }

  /**
   * Will start the writer thread if not already started. Must be called from
   * the main thread.
   */
  void startWriter();

  /**
   * If still running, will stop the writer thread.
   */
  void stopWriter();

  /**
   * Will cause the writer to iterate once over record holders and write the
   * contents to logs.
   * The writer thread must be stopped.
   */
  void writerFinalPass();

private:
  Arena _arena{100000};
  Arena _empty{0};
  // TODO: Add alignment and padding?
  std::atomic<int> _currentThread = {0};
  HolderArray _threadArray{MaxThreads};
  std::size_t _droppedThreads = {0};
  FileOutputs _fileOutputs;
  Writer _writer;
  std::thread _writerThread;
  bool _writerStarted = false;
};

Manager &getManager();
}
}

#endif
