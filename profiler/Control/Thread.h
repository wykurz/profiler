#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <Queue/Queue.h>
#include <Record/Record.h>
#include <Control/RecordManager.h>
#include <cassert>
#include <memory>
#include <mutex>
#include <vector>

namespace Profiler { namespace Control
{

    struct ThreadAllocation;

    struct Thread
    {
        Thread(const ThreadAllocation& allocation_);
        Thread(const Thread&) = delete;
        ~Thread();
        template <typename RecordType_>
        RecordManager<RecordType_>& getRecordManager();
      private:
        RecordManager<Record::Record> _recordManager;
    };

    template <>
    inline RecordManager<Record::Record>& Thread::getRecordManager<Record::Record>()
    {
        // TODO:
        //   1) Store record managers as a tuple based on list of record types
        //   2) Use C++11 mpl http://pdimov.com/cpp2/simple_cxx11_metaprogramming.html
        return _recordManager;
    }

    Thread& getThread();

    struct ThreadHolder
    {
        std::unique_lock<std::mutex> lock()
        {
            return std::unique_lock<std::mutex>(*_lock);
        }
        Thread* thread = nullptr;
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
        void setThread(Thread& thread_) const
        {
            if (_holder) _holder->thread = &thread_;
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
