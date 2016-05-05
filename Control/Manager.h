#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Control/Thread.h>
#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>

namespace Control
{

    struct Manager
    {
        static constexpr std::size_t MaxThreads = 1024;
        static constexpr std::size_t MaxRecords = (1 << 20);
        using RecordType = Record::Record;
        using NodeType = Queue::Node<RecordType>;
        Manager()
        {
            for (auto& block : arena) free.push(&block);
        }
        // TODO: How should we deal with infinite # of threads
        std::atomic<int> currentThread;
        std::array<Thread*, MaxThreads> threadBuffers;
        // TODO: Will we have multiple of those function?
        const NodeType* getNodeBase() const
        {
            return &arena[0];
        }
      private:
        std::array<Queue::Node<Record::Record>, MaxRecords> arena = {};
      public:
        Queue::Queue<Record::Record> free = {getNodeBase(), MaxRecords};
    };

    Manager& getManager();

    template <typename RecordType_>
    RecordHandle<RecordType_> RecordManager<RecordType_>::getRecord()
    {
        // TODO: Pick up first record from the block, only if not available pull from global queue
        auto record = getManager().free.pull();
        if (!record)
        {
            ++(getThread().droppedRecords);
            return RecordHandle<RecordType>();
        }
        return RecordHandle<RecordType>(record);
    }

}

#endif
