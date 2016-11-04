#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <Queue/Queue.h>
#include <Record/Record.h>
#include <Control/RecordManager.h>
#include <cassert>
#include <memory>
#include <mutex>
#include <vector>

namespace Control
{

    struct ThreadHolder;

    struct Thread
    {
        template <typename ManagerType_>
        Thread(ManagerType_& manager_)
          : _arena(manager_.addThread(*this))
        { }
        Thread(const Thread&) = delete;
        ~Thread();
        template <typename RecordType_>
        RecordManager<RecordType_>& getRecordManager();
      private:
        Arena& _arena;
        RecordManager<Record::Record> _recordManager{_arena};
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

    using ThreadArray = std::vector<ThreadHolder>;

}

#endif
