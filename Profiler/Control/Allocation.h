#ifndef _PROFILER_CONTROL_ALLOCATION_H
#define _PROFILER_CONTROL_ALLOCATION_H

#include <Profiler/Control/Arena.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Control/RecordManager.h>
#include <Profiler/Exception.h>
#include <atomic>
#include <mutex>

namespace Profiler {
namespace Control {

struct Allocation {
  Allocation(std::unique_lock<std::mutex> &&lock_, Arena &arena_,
             Holder &holder_)
      : _lock(std::move(lock_)), _arena(arena_), _holder(&holder_) {}
  Allocation() : _arena(empty()), _holder(nullptr) {}
  Arena &getArena() const { return _arena; }
  Finalizer setupHolder(RecordExtractor &recordExtractor_) const {
    if (_holder != nullptr)
      _holder->setRecordExtractor(recordExtractor_);
    return Finalizer(_holder);
  }

private:
  static Arena &empty() {
    static Arena empty(0);
    return empty;
  }
  std::unique_lock<std::mutex> _lock;
  Arena &_arena;
  Holder *const _holder;
};
} // namespace Control
} // namespace Profiler

#endif
