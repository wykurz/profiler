#ifndef RECORD_RECORD_H
#define RECORD_RECORD_H

#include <chrono>

namespace Record
{

    using TimeDelta = std::chrono::duration<double>;

    struct Record
    {
        Record() = default;
        Record(const char* name_, const TimeDelta& delta_)
          : name(name_),
            delta(delta_)
        { }
        const char* name = nullptr;
        TimeDelta delta;
    };

    template <typename RecordNode_, typename RecordManager_>
    struct RecordHandle
    {
        using RecordNode = RecordNode_;
        using RecordType = typename RecordNode::Type;
        using RecordManagerType = RecordManager_;
        RecordHandle(RecordManagerType& manager_, RecordNode* record_ = nullptr)
          : _manager(manager_),
            _record(record_)
        { }
        ~RecordHandle()
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
        RecordManagerType& _manager;
        RecordNode* const _record;
    };

}

#endif
