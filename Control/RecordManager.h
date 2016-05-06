#ifndef CONTROL_RECORDMANAGER_H
#define CONTROL_RECORDMANAGER_H

#include <Control/Manager.h>
#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>

namespace Control
{

    template <typename RecordType_>
    struct RecordManager
    {
        using RecordType = RecordType_;
        using RecordNode = Queue::Node<RecordType>;
        using This = RecordManager<RecordType>;
        using RecordHandleType = Record::RecordHandle<RecordNode, This>;
        using RecordQueue = Queue::Queue<RecordType>;
        RecordManager(const RecordNode* baseNode_, std::size_t size_)
          : dirty(baseNode_, size_)
        { }
        RecordManager(const This&) = delete;
        RecordHandleType getRecord()
        {
            // TODO: Should we make this a template function? Or the whole class?
            auto recordNode = getManager().getFreeRecordNode();
            if (!recordNode)
            {
                ++droppedRecords;
                return RecordHandleType(*this);
            }
            return RecordHandleType(*this, recordNode);
        }
        void retireRecord(RecordNode* record_)
        {
            dirty.push(record_);
        }
        const RecordQueue& getDirtyRecords() const
        {
            return dirty;
        }
      private:
        std::size_t droppedRecords = 0;
        RecordQueue dirty;
    };

}

#endif
