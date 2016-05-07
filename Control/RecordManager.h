#ifndef CONTROL_RECORDMANAGER_H
#define CONTROL_RECORDMANAGER_H

#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>

namespace Control
{

    template <typename Record_>
    struct RecordManager
    {
        using This = RecordManager<Record_>;
        using RecordStorageType = Record::RecordStorage<Record_>;
        using RecordQueue = typename RecordStorageType::RecordQueue;
        using RecordNode = typename RecordStorageType::RecordNode;
        using RecordType = typename RecordStorageType::RecordType;
        struct RecordHolder
        {
            RecordHolder(This& manager_, RecordNode* record_ = nullptr)
              : _manager(manager_),
                _record(record_)
            { }
            ~RecordHolder()
            {
                if (this->isValid()) _manager.retireRecord(_record);
            }
            bool isValid() const
            {
                return nullptr != _record;
            }
            RecordType& getRecord()
            {
                assert(_record);
                return _record->value;
            }
          private:
            This& _manager;
            RecordNode* const _record;
        };
        RecordManager(RecordStorageType& recordStorage_)
          : _recordStorage(recordStorage_),
            _dirty(recordStorage_.getNodeBase(), recordStorage_.size())
        { }
        RecordManager(const This&) = delete;
        RecordHolder getRecord()
        {
            auto recordNode = _recordStorage.getFreeRecordNode();
            if (!recordNode)
            {
                ++_droppedRecords;
                return RecordHolder(*this);
            }
            return RecordHolder(*this, recordNode);
        }
        void retireRecord(RecordNode* record_)
        {
            _dirty.push(record_);
        }
        RecordNode* extractDirtyRecords()
        {
            return _dirty.extract();
        }
      private:
        RecordStorageType& _recordStorage;
        // TODO: add padding
        std::size_t _droppedRecords = 0;
        RecordQueue _dirty;
    };

}

#endif
