#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <Queue/Queue.h>
#include <Record/Record.h>
#include <Control/RecordManager.h>
#include <cassert>
#include <mutex>

namespace Control
{

    struct Thread
    {
        using RecordStorageType = Record::RecordStorage<Record::Record>;
        using RecordManagerType = RecordManager<Record::Record>;
        Thread();
        Thread(const Thread&) = delete;
        ~Thread();
        template <typename RecordType_>
        RecordManager<RecordType_>& getRecordManager();
      private:
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
        std::mutex lock;
        Thread* thread;
    };

    using ThreadArray = std::vector<ThreadHolder>;

}

#endif
