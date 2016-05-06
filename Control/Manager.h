#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

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
        static constexpr std::size_t MaxRecords = (1 << 20);
        using RecordType = Record::Record;
        using RecordNode = Queue::Node<RecordType>;
        Manager()
        {
            for (auto& block : arena) _free.push(&block);
        }
        // TODO: How should we deal with infinite # of threads
        std::atomic<int> currentThread;
        // TODO: Will we have multiple of those function?
        const RecordNode* getNodeBase() const
        {
            return &arena[0];
        }
        void addThread(Thread& thread_)
        {
            bool found = false;
            auto count = MaxSlotSearches;
            while (!found && 0 < count--) {
                auto id = currentThread++;
                std::unique_lock<std::mutex> lk(_threadBuffers[id].lock);
                if (_threadBuffers[id].thread) continue;
                _threadBuffers[id].thread = &thread_;
                found = true;
            }
            if (!found) ++_droppedThreads;
        }
        RecordNode* getFreeRecordNode()
        {
            return _free.pull();
        }
      private:
        struct Holder
        {
            Thread* thread;
            std::mutex lock;
        };
        std::array<Holder, MaxThreads> _threadBuffers;
        std::size_t _droppedThreads = 0;
        std::array<Queue::Node<Record::Record>, MaxRecords> arena = {};
        Queue::Queue<Record::Record> _free = {getNodeBase(), MaxRecords};
    };

    Manager& getManager();

}

#endif
