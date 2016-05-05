#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>

namespace Control
{

    template <typename RecordType_>
    struct RecordHandle
    {
        using RecordType = RecordType_;
        using RecordNode = Queue::Node<RecordType>;
        RecordHandle(RecordNode* record_ = nullptr)
          : _record(record_)
        { }
        ~RecordHandle();
        bool isValid() const;
        RecordType& getRecord()
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
        RecordHandle<RecordType> getRecord();
        Queue::Queue<RecordType> dirty;
    };

    struct Thread
    {
        Thread();
        Thread(const Thread&) = delete;
        ~Thread();
        template <typename RecordType_>
        RecordManager<RecordType_>& getRecordManager();
        std::size_t droppedRecords = 0;
      private:
        std::size_t _maxRecords;
        RecordManager<Record::Record> _recordManager;
    };

    Thread& getThread();

    template <typename RecordType_>
    RecordHandle<RecordType_>::~RecordHandle<RecordType_>()
    {
        if (this->isValid()) getThread().getRecordManager<RecordType>().dirty.push(_record);
    }

    template <typename RecordType_>
    bool RecordHandle<RecordType_>::isValid() const
    {
        return nullptr != _record;
    }

}

#endif
