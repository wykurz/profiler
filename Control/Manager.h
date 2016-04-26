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
        RecordManager() = default;
        RecordManager(const This&) = delete;
        RecordHolder<RecordType> getRecord();
        void commitRecord(RecordNode& record_);
      private:
        Queue::Queue<RecordType> _dirty;
    };

    struct Thread
    {
        Thread();
        Thread(const Thread&) = delete;
        template <typename RecordType_>
        RecordManager<RecordType_>& getRecordManager();
        std::size_t droppedRecords = 0;
      private:
        RecordManager<Record::Record> _recordManager;
    };

    Thread& getThread();

    template <typename RecordType_>
    RecordHolder<RecordType_>::~RecordHolder<RecordType>()
    {
        if (this->isValid()) getThread().getRecordManager<RecordType>().commitRecord(*_record);
    }

    template <typename RecordType_>
    bool RecordHolder<RecordType_>::isValid() const
    {
        return nullptr != _record;
    }

    struct Manager
    {
        static constexpr std::size_t MaxThreads = 1024;
        Manager()
        {
            for (auto& block : arena) free.push(&block);
        }
        // TODO: How should we deal with infinite # of threads
        std::atomic<int> currentThread;
        std::array<Thread*, MaxThreads> threadBuffers;
        static constexpr std::size_t MaxRecords = (1 << 20);
        std::array<Queue::Node<Record::Record>, MaxRecords> arena{};
        Queue::Queue<Record::Record> free;
    };

    Manager& getManager();

    template <typename RecordType_>
    RecordHolder<RecordType_> RecordManager<RecordType_>::getRecord()
    {
        // TODO: Pick up first record from the block, only if not available pull from global queue
        auto record = getManager().free.pull();
        if (!record)
        {
            ++getThread().droppedRecords;
            return RecordHolder<RecordType>();
        }
        return RecordHolder<RecordType>(record);
    }

    template <typename RecordType_>
    void RecordManager<RecordType_>::commitRecord(RecordNode& record_)
    {
        _dirty.push(&record_);
    }

}

#endif
