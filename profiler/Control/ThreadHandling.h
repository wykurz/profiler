#ifndef CONTROL_THREADHANDLING_H
#define CONTROL_THREADHANDLING_H

#include <Control/RecordManager.h>
#include <memory>
#include <mutex>
#include <vector>

namespace Profiler { namespace Control
{

    struct ThreadHolder
    {
        std::unique_lock<std::mutex> lock()
        {
            return std::unique_lock<std::mutex>(*_lock);
        }
        RecordExtractor* recordExtractor = nullptr;
      private:
        std::unique_ptr<std::mutex> _lock = std::make_unique<std::mutex>();
    };

    struct ThreadAllocation
    {
        ThreadAllocation(std::unique_lock<std::mutex>&& lock_, Arena& arena_, ThreadHolder& holder_)
          : _lock(std::move(lock_)), _arena(arena_), _holder(&holder_)
        { }
        ThreadAllocation()
          : _arena(empty()), _holder(nullptr)
        { }
        Arena& getArena() const
        {
            return _arena;
        }
        void setRecordExtractor(RecordExtractor& recordExtractor_) const
        {
            if (_holder) _holder->recordExtractor = &recordExtractor_;
        }
      private:
        static Arena& empty()
        {
            static Arena empty(0);
            return empty;
        }
        std::unique_lock<std::mutex> _lock;
        Arena& _arena;
        ThreadHolder* const _holder;
    };

    using ThreadArray = std::vector<ThreadHolder>;

}
}

#endif
