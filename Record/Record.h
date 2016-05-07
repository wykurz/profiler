#ifndef RECORD_RECORD_H
#define RECORD_RECORD_H

#include <Queue/Queue.h>
#include <array>
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

    template <typename Record_>
    struct RecordStorage
    {
        using RecordQueue = Queue::Queue<Record_>;
        using RecordNode = typename RecordQueue::NodeType;
        using RecordType = Record_;
        RecordStorage(std::size_t size_)
          : _arena(size_),
            _free(this->getNodeBase(), this->size())
        {
            for (auto& block : _arena) _free.push(&block);
        }
        RecordStorage(const RecordStorage&) = delete;
        std::size_t size() const
        {
            return _arena.size();
        }
        const RecordNode* getNodeBase() const
        {
            return &_arena[0];
        }
        RecordNode* getFreeRecordNode()
        {
            return _free.pull();
        }
      private:
        std::vector<RecordNode> _arena;
        Queue::Queue<Record_> _free;
    };


}

#endif
