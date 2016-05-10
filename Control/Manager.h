#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Control/RecordManager.h>
#include <Control/Writer.h>
#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>
#include <condition_variable>

namespace Control
{

    struct Thread;

    struct Manager
    {
        static constexpr std::size_t MaxThreads = 1024;
        static constexpr std::size_t MaxSlotSearches = MaxThreads;
        static constexpr std::size_t NumRecords = (1 << 20);
        using RecordStorageType = Record::RecordStorage<Record::Record>;
        using RecordNode = typename RecordStorageType::RecordNode;
        using RecordType = typename RecordStorageType::RecordType;
        Manager() = default;
        Manager(const Manager&) = delete;
        ThreadHolder* addThread(Thread& thread_);
        // TODO: Template on record type?
        RecordStorageType& getRecordStorage();
      private:
        // TODO: Add alignment and padding?
        std::atomic<int> _currentThread = {0};
        ThreadArray _threadArray{MaxThreads};
        std::size_t _droppedThreads = {0};
        RecordStorageType _recordStorage{NumRecords};
        Writer _writer{"blah"};
    };

    Manager& getManager();

}

#endif
