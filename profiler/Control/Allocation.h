#ifndef CONTROL_ALLOCATION_H
#define CONTROL_ALLOCATION_H

#include <Control/Arena.h>
#include <Control/Holder.h>
#include <Control/RecordManager.h>
#include <mutex>

namespace Profiler { namespace Control
{

    struct Finalizer
    {
        Finalizer(Holder* holder_)
          : _holder(holder_)
        { }
        ~Finalizer()
        {
            if (_holder) {
                auto lk = _holder->lock();
                _holder->finalize();
            }
        }
      private:
        Holder* const _holder;
    };

    struct Allocation
    {
        Allocation(std::unique_lock<std::mutex>&& lock_, Arena& arena_, Holder& holder_)
          : _lock(std::move(lock_)), _arena(arena_), _holder(&holder_)
        { }
        Allocation()
          : _arena(empty()), _holder(nullptr)
        { }
        Arena& getArena() const
        {
            return _arena;
        }
        Finalizer setRecordExtractor(RecordExtractor& recordExtractor_) const
        {
            if (_holder) _holder->setRecordExtractor(recordExtractor_);
            return Finalizer(_holder);
        }
      private:
        static Arena& empty()
        {
            static Arena empty(0);
            return empty;
        }
        std::unique_lock<std::mutex> _lock;
        Arena& _arena;
        Holder* const _holder;
    };

}
}

#endif
