#ifndef _PROFILER_CONTROL_MANAGER_H
#define _PROFILER_CONTROL_MANAGER_H

#include <Profiler/Config.h>
#include <Profiler/Control/Allocation.h>
#include <Profiler/Control/Arena.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Writer/Processor.h>
#include <Profiler/Exception.h>
#include <atomic>
#include <thread>

namespace Profiler {
namespace Control {

struct Manager {
  Manager(std::size_t instanceId_)
      : _instanceId(instanceId_)
    { }
  virtual ~Manager() { }
  template <typename RecordType_> Allocation<RecordType_> addThreadRecords() {
    std::size_t id = _currentThread++;
    // TODO(mateusz): Add stress tests with tons of threads...
    auto holderPtr = static_cast<Holder<RecordType_>*>(findHolder(typeid(RecordType_)));
    if (holderPtr)
      return {id, _arena, *holderPtr};
    ++_droppedThreads;
    return {};
  }

  /**
   * Will start the writer thread if not already started. Must be called from
   * the main thread.
   */
  virtual void startProcessor() = 0;

  /**
   * If still running, will stop the writer thread.
   */
  virtual void stopProcessor() = 0;

  virtual bool isProcessorStarted() const = 0;

  /**
   * Will cause the writer to iterate once over record holders and write the
   * contents to logs. The writer thread must be stopped.
   */
  virtual void writerFinalPass() = 0;

  /**
   * Capture per-process instance name. It's particularly useful when
   * identifying the begin of an event record if it crosses process boundaries.
   */
  std::size_t id() { return _instanceId; }

 protected:
  virtual void* findHolder(std::type_index recordTypeId_) = 0;

 private:
  Arena _arena{100000};
  Arena _empty{0};
  const std::size_t _instanceId;
  std::atomic<std::uint64_t> _currentThread = {0};
  std::atomic<std::uint64_t> _droppedThreads = {0};
};

template <typename ConfigType_>
struct ManagerImpl : Manager {
  using ConfigType = ConfigType_;
  using RecordList = typename ConfigType::RecordList;
  explicit ManagerImpl(const ConfigType& config_)
      : Manager(config_.instanceId), _config(config_) { }
  ManagerImpl(const Manager &) = delete;
  ~ManagerImpl() override { stopProcessor(); }

  void startProcessor() override {
    if (_processorStarted)
      return;
    _processorThread = std::thread([this]() { this->_processor.run(); });
    _processorStarted = true;
  }

  void stopProcessor() override {
    DLOG("Stopping writer!");
    if (!_processorStarted)
      return;
    _processor.stop();
    if (_processorThread.joinable())
      _processorThread.join();
    _processorStarted = false;
    writerFinalPass();
  }

  bool isProcessorStarted() const override { return _processorStarted; }

  void writerFinalPass() override  {
    PROFILER_ASSERT(!_processorStarted);
    _processor.finalPass();
  }
 protected:
  void* findHolder(std::type_index recordTypeId_) override {
    return _holderArray.findHolder(recordTypeId_);
  }

 private:
  template <typename RecordType_> static void setupStream(std::ostream &out_) {
    const std::string &recordTypeName = typeid(RecordType_).name();
    DLOG("Setup: " << recordTypeName.size() << " " << recordTypeName << " "
                   << std::size_t(&out_))
    const std::size_t &nameSize = recordTypeName.size();
    out_.write(reinterpret_cast<const char *>(&nameSize), sizeof(nameSize));
    out_ << recordTypeName;
    RecordType_::encodePreamble(out_);
  }

  // TODO(mateusz): Add alignment and padding?
  const ConfigType _config;
  HolderArray<RecordList> _holderArray;
  Writer::Processor<ConfigType> _processor{_config, _holderArray};
  std::thread _processorThread;
  bool _processorStarted = false;
};

namespace Internal {

inline Manager *&managerInstancePtr() {
  static Manager* manager = nullptr;
  return manager;
}
} // namespace Internal

template <typename ConfigType_>
void setManager(const ConfigType_& config_) {
  static ManagerImpl<ConfigType_> manager(config_);
  if (Internal::managerInstancePtr())
    PROFILER_RUNTIME_ERROR("Profiler already set up!");
  Internal::managerInstancePtr() = &manager;
}

inline Manager &getManager() {
  auto managerPtr = Internal::managerInstancePtr();
  if (!managerPtr || !managerPtr->isProcessorStarted())
    PROFILER_RUNTIME_ERROR("You must setup Profiler first!");
  return *Internal::managerInstancePtr();
}

} // namespace Control
} // namespace Profiler

#endif
