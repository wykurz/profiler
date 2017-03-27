#ifndef _PROFILER_CONTROL_THREADRECORDS_H
#define _PROFILER_CONTROL_THREADRECORDS_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/RecordManager.h>
#include <Profiler/Record/Records.h>
#include <memory>
#include <mutex>
#include <vector>

namespace Profiler {
namespace Control {

template <typename Record_> struct ThreadRecords {
  using RecordManagerType = RecordManager<Record_>;
  explicit ThreadRecords(const Allocation &allocation_)
      : id(allocation_.id), _recordManager(allocation_.getArena()),
        _finalizer(allocation_.setupHolder(_recordManager)) {}
  ThreadRecords(const ThreadRecords &) = delete;
  RecordManagerType &getRecordManager() { return _recordManager; }
  const std::size_t id;

private:
  RecordManager<Record_> _recordManager;
  Finalizer _finalizer;
};

template <typename Record_> ThreadRecords<Record_> &getThreadRecords() {
  thread_local ThreadRecords<Record_> threadRecords(
      getManager().addThreadRecords<Record_>());
  return threadRecords;
}

// TODO(mateusz): measure and document the cost of this call
template <typename RecordTypes_ = Mpl::TypeList<>> void primeThisThread() {
  auto requestRecordType = [](auto dummy_) {
    using RecordType = typename decltype(dummy_)::Type;
    getThreadRecords<RecordType>();
  };
  Mpl::apply<Record::NativeRecords>(requestRecordType);
  Mpl::apply<RecordTypes_>(requestRecordType);
}
} // namespace Control
} // namespace Profiler

#endif
