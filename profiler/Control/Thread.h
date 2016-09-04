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
        using RecordStorageType = Record::RecordStorage<Record::Record>;
        using RecordManagerType = RecordManager<Record::Record>;
        template <typename ManagerType_>
        Thread(ManagerType_& manager_)
          : _holder(manager_.addThread(*this)),
            _recordManager(manager_.getRecordStorage())
        {
            if (!_holder) { } // TODO - handle failure case
        }
        Thread(const Thread&) = delete;
        ~Thread();
        template <typename RecordType_>
        RecordManager<RecordType_>& getRecordManager();
      private:
        ThreadHolder* _holder;
        RecordManagerType _recordManager;
    };

    template <>
    inline RecordManager<Record::Record>& Thread::getRecordManager<Record::Record>()
    {
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
        std::unique_ptr<std::mutex> _lock{new std::mutex()};
    };

    using ThreadArray = std::vector<ThreadHolder>;

}

#endif
