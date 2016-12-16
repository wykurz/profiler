#ifndef CONTROL_RECORDMANAGER_H
#define CONTROL_RECORDMANAGER_H

#include <Profiler/Control/Arena.h>
#include <Profiler/Exception/Exception.h>
#include <Profiler/Log/Log.h>
#include <Profiler/Queue/Queue.h>
#include <Profiler/Record/Record.h>
#include <iostream>
#include <memory>
#include <typeindex>
#include <vector>

namespace Profiler { namespace Control
{

    struct RecordExtractor
    {
        ~RecordExtractor() = default;
        virtual std::type_index getRecordTypeHash() const = 0;
        virtual void streamDirtyRecords(std::ostream& out_) = 0;
        virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() = 0;
    };

namespace Internal
{

    template <typename Record_>
    struct ManagerBase : RecordExtractor
    {
        using Record = Record_;
        virtual std::type_index getRecordTypeHash() const override
        {
            return std::type_index(typeid(Record));
        }
    };

}

    template <typename Record_>
    struct SimpleExtractor : Internal::ManagerBase<Record_>
    {
        using Record = Record_;
        using Queue = Queue::Queue<Record>;
        using Node = typename Queue::Node;

        SimpleExtractor(Node* const records_)
          : _records(records_)
        { }
        virtual void streamDirtyRecords(std::ostream& out_) override
        {
            auto recordNode = _records;
            while (recordNode) {
                out_ << recordNode->value;
                recordNode = recordNode->getNext();
            }
        }
        virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() override
        {
            throw Exception::LogicError("Attempting to finalize SimpleExtractor");
        }
      private:
        Node* const _records;
    };

    template <typename Record_>
    struct RecordManager : Internal::ManagerBase<Record_>
    {
        using Record = Record_;
        using This = RecordManager<Record>;
        using Queue = Queue::Queue<Record>;
        using Node = typename Queue::Node;

        struct RecordHolder
        {
            RecordHolder(This& manager_, Node* node_ = nullptr)
              : _manager(manager_),
                _node(node_)
            { }
            ~RecordHolder()
            {
                if (this->isValid()) _manager.retireRecord(*_node);
            }
            bool isValid() const
            {
                return nullptr != _node;
            }
            Record& getRecord()
            {
                assert(_node);
                return _node->value;
            }
          private:
            This& _manager;
            Node* const _node;
        };

        RecordManager(Arena& arena_)
          : _arena(arena_),
            _dirty(arena_.basePtr<Node>())
        { }
        RecordManager(const This&) = delete;

        RecordHolder getRecord()
        {
            if (!_currentBlock || _currentBlock->size() <= _nextRecord)
            {
                _currentBlock = _arena.acquire<Node>();
                _nextRecord = 0;
            }
            if (!_currentBlock)
            {
                ++_droppedRecords;
                return RecordHolder(*this);
            }
            return RecordHolder(*this, &(*_currentBlock)[_nextRecord++]);
        }

        void retireRecord(Node& node_)
        {
            DLOG("Retire record " << &node_);
            _dirty.push(&node_);
        }

        virtual void streamDirtyRecords(std::ostream& out_) override
        {
            auto recordNode = extractDirtyRecords();
            while (recordNode) {
                out_ << recordNode->value;
                recordNode = recordNode->getNext();
            }
        }

        virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() override
        {
            return std::make_unique<SimpleExtractor<Record> >(extractDirtyRecords());
        }

      private:
        Node* extractDirtyRecords()
        {
            DLOG("extractDirtyRecords");
            return _dirty.extract();
        }

        Arena& _arena;
        // TODO: add padding
        Arena::Block<Node>* _currentBlock = nullptr;
        std::size_t _nextRecord = 0;
        std::size_t _droppedRecords = 0;
        Queue _dirty;
    };

}
}

#endif
