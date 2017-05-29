#ifndef _PROFILER_CONTROL_THREADRECORDS_H
#define _PROFILER_CONTROL_THREADRECORDS_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/RecordManager.h>
#include <memory>
#include <mutex>
#include <vector>

namespace Profiler {
namespace Control {

template <typename RecordType_> struct ThreadRecords {
  using RecordType = RecordType_;
  using RecordManagerType = RecordManager<RecordType>;
  explicit ThreadRecords(const Allocation<RecordType> &allocation_)
      : id(allocation_.id), _recordManager(allocation_.getArena()),
        _finalizer(allocation_.setupHolder(_recordManager)) {
    DLOG("ThreadRecords construction complete");
  }
  ThreadRecords(const ThreadRecords &) = delete;
  RecordManagerType &getRecordManager() { return _recordManager; }
  const std::size_t id;

private:
  RecordManager<RecordType> _recordManager;
  Finalizer<RecordType> _finalizer;
};

template <typename RecordType_> ThreadRecords<RecordType_> &getThreadRecords() {
  thread_local ThreadRecords<RecordType_> threadRecords(
      getManager().addThreadRecords<RecordType_>());
  return threadRecords;
}

// TODO(mateusz): measure and document the cost of this call
template <typename RecordTypes_> void primeThisThread() {
  auto requestRecordType = [](auto dummy_) {
    using RecordType = typename decltype(dummy_)::Type;
    getThreadRecords<RecordType>();
  };
  Mpl::apply<RecordTypes_>(requestRecordType);
}
} // namespace Control
} // namespace Profiler

#endif
