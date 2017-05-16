#ifndef _PROFILER_CONTROL_MANAGER_H
#define _PROFILER_CONTROL_MANAGER_H

#include <Profiler/Config.h>
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

  explicit Manager(Config config_, bool startWriter_ = true)
      : _config(std::move(config_)) {
    if (startWriter_) {
      _writerThread = std::thread([this]() { this->_writer.run(); });
      _writerStarted = true;
    }
  }
  Manager(const Manager &) = delete;
  ~Manager() { stopWriter(); }

  template <typename Record_> Allocation addThreadRecords() {
    int count = MaxThreads;
    while (0 < count--) {
      std::size_t id = _currentThread++;
      // TODO(mateusz): Add stress tests with tons of threads...
      auto &holder = _threadArray[id % _threadArray.size()];
      auto lk = holder.lock();
      if (!holder.isEmpty())
        continue;
      auto out = _fileOutputs.newOutput(id);
      setupStream<Record_>(out->get());
      holder.setOut(std::move(out));
      return {id, std::move(lk), _arena, holder};
    }
    ++_droppedThreads;
    return {};
  }

  /**
   * Will start the writer thread if not already started. Must be called from
   * the main thread.
   */
  void startWriter() {
    if (_writerStarted)
      return;
    _writerThread = std::thread([this]() { this->_writer.run(); });
    _writerStarted = true;
  }

  /**
   * If still running, will stop the writer thread.
   */
  void stopWriter() {
    if (!_writerStarted)
      return;
    _writer.stop();
    if (_writerThread.joinable())
      _writerThread.join();
    _writerStarted = false;
    writerFinalPass();
  }

  /**
   * Will cause the writer to iterate once over record holders and write the
   * contents to logs. The writer thread must be stopped.
   */
  void writerFinalPass() {
    PROFILER_ASSERT(!_writerStarted);
    _writer.finalPass();
  }

  /**
   * Capture per-process instance name. It's particularly useful when
   * identifying the begin of an event record if it crosses process boundaries.
   */
  const std::size_t &id() { return _config.instanceId; }

private:
  template <typename Record_> static void setupStream(std::ostream &out_) {
    const std::string &recordTypeName = typeid(Record_).name();
    DLOG("Setup: " << recordTypeName.size() << " " << recordTypeName << " "
                   << std::size_t(&out_))
    const std::size_t &nameSize = recordTypeName.size();
    out_.write(reinterpret_cast<const char *>(&nameSize), sizeof(nameSize));
    out_ << recordTypeName;
    Record_::encodePreamble(out_);
  }

  Arena _arena{100000};
  Arena _empty{0};
  // TODO(mateusz): Add alignment and padding?
  std::atomic<int> _currentThread = {0};
  HolderArray _threadArray{MaxThreads};
  std::size_t _droppedThreads = {0};
  const Config _config;
  FileOutputs _fileOutputs{_config};
  Writer _writer{_threadArray, std::chrono::microseconds(100000)};
  std::thread _writerThread;
  bool _writerStarted = false;
};

inline Manager &getManager() {
  static Manager manager(Config::getConfig());
  return manager;
}

} // namespace Control
} // namespace Profiler

#endif
