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
    auto holder = static_cast<Holder<RecordType_>*>(findHolder(typeid(RecordType_)));
    if (holder)
      return {id, holder->adoptLock(), _arena, *holder};
    ++_droppedThreads;
    return {};
  }

  /**
   * Will start the writer thread if not already started. Must be called from
   * the main thread.
   */
  virtual void startWriter() = 0;

  /**
   * If still running, will stop the writer thread.
   */
  virtual void stopWriter() = 0;

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
  ~ManagerImpl() override { stopWriter(); }

  void startWriter() override {
    if (_writerStarted)
      return;
    _writerThread = std::thread([this]() { this->_writer.run(); });
    _writerStarted = true;
  }

  void stopWriter() override {
    if (!_writerStarted)
      return;
    _writer.stop();
    if (_writerThread.joinable())
      _writerThread.join();
    _writerStarted = false;
    writerFinalPass();
  }

  void writerFinalPass() override  {
    PROFILER_ASSERT(!_writerStarted);
    _writer.finalPass();
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
  Writer::Processor<ConfigType> _writer{_config, _holderArray};
  // FileOutputs _fileOutputs{_config};
  std::thread _writerThread;
  bool _writerStarted = false;
};

namespace Internal {

inline Manager &managerInstance(Manager *manager_ = nullptr) {
  static Manager* manager(manager_);
  if (!manager) PROFILER_RUNTIME_ERROR("You must setup Profiler first!");
  return *manager;
}
} // namespace Internal

template <typename ConfigType_>
void setManager(const ConfigType_& config_) {
  static ManagerImpl<ConfigType_> manager(config_);
  Internal::managerInstance(&manager);
}

inline Manager &getManager() {
  return Internal::managerInstance();
}

} // namespace Control
} // namespace Profiler

#endif
