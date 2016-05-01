#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>

namespace Control
{

    template <typename RecordType_>
    struct RecordHolder
    {
        using RecordType = RecordType_;
        using RecordNode = Queue::Node<RecordType>;
        RecordHolder(RecordNode* record_ = nullptr)
          : _record(record_)
        { }
        ~RecordHolder();
        bool isValid() const;
        RecordType& get()
        {
            assert(_record);
            return _record->value;
        }
      private:
        RecordNode* const _record;
    };

    template <typename RecordType_>
    struct RecordManager
    {
        using RecordType = RecordType_;
        using RecordNode = Queue::Node<RecordType>;
        using This = RecordManager<RecordType>;
        RecordManager(const RecordNode* baseNode_, std::size_t size_)
          : dirty(baseNode_, size_)
        { }
        RecordManager(const This&) = delete;
        RecordHolder<RecordType> getRecord();
        Queue::Queue<RecordType> dirty;
    };

    struct Thread
    {
        Thread();
        Thread(const Thread&) = delete;
        template <typename RecordType_>
        RecordManager<RecordType_>& getRecordManager();
        std::size_t droppedRecords = 0;
      private:
        std::size_t _maxRecords;
        RecordManager<Record::Record> _recordManager;
    };

    Thread& getThread();

    template <typename RecordType_>
    RecordHolder<RecordType_>::~RecordHolder<RecordType_>()
    {
        if (this->isValid()) getThread().getRecordManager<RecordType>().dirty.push(_record);
    }

    template <typename RecordType_>
    bool RecordHolder<RecordType_>::isValid() const
    {
        return nullptr != _record;
    }

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
        std::array<Queue::Node<Record::Record>, MaxRecords> arena{};
      public:
        Queue::Queue<Record::Record> free{getNodeBase(), MaxRecords};
    };

    Manager& getManager();

    template <typename RecordType_>
    RecordHolder<RecordType_> RecordManager<RecordType_>::getRecord()
    {
        // TODO: Pick up first record from the block, only if not available pull from global queue
        auto record = getManager().free.pull();
        if (!record)
        {
            ++(getThread().droppedRecords);
            return RecordHolder<RecordType>();
        }
        return RecordHolder<RecordType>(record);
    }

}

#endif
