#ifndef CONTROL_RECORDMANAGER_H
#define CONTROL_RECORDMANAGER_H

#include <Profiler/Control/Arena.h>
#include <Profiler/Exception/Exception.h>
#include <Profiler/Log/Log.h>
#include <Profiler/Queue/Queue.h>
#include <Profiler/Record/Record.h>
#include <iostream>
#include <memory>
#include <vector>

namespace Profiler { namespace Control
{

    struct RecordExtractor
    {
        virtual ~RecordExtractor() = default;
        virtual void streamDirtyRecords(std::ostream& out_) = 0;
        virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() = 0;
    };

namespace Internal
{

    template <typename Record_>
    struct ExtractorBase : RecordExtractor
    {
        using RecordType = Record_;
        using Queue = Queue::Queue<RecordType>;
        using Node = typename Queue::Node;
      protected:
        void doStreamDirtyRecords(std::ostream& out_, Node* recordNode_)
        {
            while (recordNode_) {
                DLOG("Streaming " << recordNode_);
                PROFILER_ASSERT(recordNode_->value.dirty());
                out_ << recordNode_->value;
                recordNode_ = recordNode_->getNext();
            }
        }
    };

}

    template <typename Record_>
    struct SimpleExtractor : Internal::ExtractorBase<Record_>
    {
        using Base = Internal::ExtractorBase<Record_>;
        using RecordType = Record_;
        using Queue = Queue::Queue<RecordType>;
        using Node = typename Queue::Node;

        SimpleExtractor(Node* const records_)
          : _records(records_)
        { }
        virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() override
        {
            throw Exception::LogicError("Attempting to finalize SimpleExtractor");
        }
        virtual void streamDirtyRecords(std::ostream& out_) override
        {
            Base::doStreamDirtyRecords(out_, _records);
        }
      private:
        Node* const _records;
    };

    template <typename Record_>
    struct RecordManager : Internal::ExtractorBase<Record_>
    {
        using Base = Internal::ExtractorBase<Record_>;
        using RecordType = Record_;
        using This = RecordManager<RecordType>;
        using Queue = Queue::Queue<RecordType>;
        using Node = typename Queue::Node;

        struct RecordHolder
        {
            RecordHolder(This& manager_, Node* node_ = nullptr)
              : _manager(manager_),
                _node(node_)
            {
                DLOG("From Node* " << node_);
            }
            ~RecordHolder()
            {
                if (_node) _manager.retireRecord(*_node);
            }
            bool isValid() const
            {
                return nullptr != _node;
            }
            RecordType& getRecord()
            {
                PROFILER_ASSERT(_node);
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
                DLOG("Arena acquire, block: " << _currentBlock);
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
            PROFILER_ASSERT(node_.value.dirty());
            DLOG("Retire record " << &node_);
            _dirty.push(&node_);
        }

        virtual void streamDirtyRecords(std::ostream& out_) override
        {
            Base::doStreamDirtyRecords(out_, extractDirtyRecords());
        }

        virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() override
        {
            return std::make_unique<SimpleExtractor<RecordType> >(extractDirtyRecords());
        }

      private:
        Node* extractDirtyRecords()
        {
            auto node = _dirty.extract();
            DLOG("extractDirtyRecords, node " << node);
            return node;
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
