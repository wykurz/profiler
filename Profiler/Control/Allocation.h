#ifndef CONTROL_ALLOCATION_H
#define CONTROL_ALLOCATION_H

#include <Profiler/Control/Arena.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Control/RecordManager.h>
#include <Profiler/Exception/Exception.h>
#include <atomic>
#include <mutex>

namespace Profiler { namespace Control
{

    struct Allocation
    {
        Allocation(std::unique_lock<std::mutex>&& lock_, Arena& arena_, Holder& holder_,
                   const OutputFactory& fileOutputs_)
          : _id(_globalId.fetch_add(1, std::memory_order_release)), _lock(std::move(lock_)),
            _arena(arena_), _holder(&holder_), _fileOutputs(&fileOutputs_)
        { }
        Allocation()
          : _id(-1), _arena(empty()), _holder(nullptr), _fileOutputs(nullptr)
        { }
        Arena& getArena() const
        {
            return _arena;
        }
        Finalizer setupHolder(RecordExtractor& recordExtractor_) const
        {
            if (_holder) _holder->setup(recordExtractor_, _fileOutputs->newOutput(_id, recordExtractor_.getRecordId()));
            return Finalizer(_holder);
        }
        Holder::Id getId() const
        {
            return _id;
        }
      private:
        static Arena& empty()
        {
            static Arena empty(0);
            return empty;
        }
        static std::atomic<Holder::Id> _globalId;
        const Holder::Id _id;
        std::unique_lock<std::mutex> _lock;
        Arena& _arena;
        Holder* const _holder;
        const OutputFactory* const _fileOutputs;
    };

}
}

#endif
