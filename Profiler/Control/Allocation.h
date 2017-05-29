#ifndef _PROFILER_CONTROL_ALLOCATION_H
#define _PROFILER_CONTROL_ALLOCATION_H

#include <Profiler/Control/Arena.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Control/RecordManager.h>
#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <atomic>
#include <mutex>

namespace Profiler {
namespace Control {

template <typename RecordType_>
struct Allocation {
  using RecordType = RecordType_;
  Allocation(std::size_t id_, Arena &arena_, Holder<RecordType> &holder_)
      : id(id_), _lock(holder_.adoptLock()), _arena(arena_), _holder(&holder_) {}
  Allocation() : id(-1), _arena(empty()), _holder(nullptr) {}
  Arena &getArena() const { return _arena; }
  Finalizer<RecordType> setupHolder(RecordManager<RecordType> &recordManager_) const {
    if (_holder != nullptr)
      _holder->setRecordManager(recordManager_);
    return Finalizer<RecordType>(_holder);
  }
  const std::size_t id;

private:
  static Arena &empty() {
    static Arena empty(0);
    return empty;
  }
  std::unique_lock<std::mutex> _lock;
  Arena &_arena;
  Holder<RecordType> *const _holder;
};
} // namespace Control
} // namespace Profiler

#endif
