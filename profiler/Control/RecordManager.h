#ifndef CONTROL_RECORDMANAGER_H
#define CONTROL_RECORDMANAGER_H

#include <Control/Arena.h>
#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>

namespace Control
{

    template <typename Record_>
    struct RecordManager
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
            if (!_currentBlock || _currentBlock->size() < _nextRecord)
            {
                _currentBlock = _arena.acquire<Node>();
                _nextRecord = 0;
            }
            if (!_currentBlock)
            {
                ++_droppedRecords;
                return RecordHolder(*this);
            }
            return RecordHolder(*this, &(*_currentBlock)[_nextRecord]);
        }

        void retireRecord(Node& node_)
        {
            _dirty.push(&node_);
        }

        Node* extractDirtyRecords()
        {
            return _dirty.extract();
        }

      private:
        Arena& _arena;
        // TODO: add padding
        Arena::Block<Node>* _currentBlock = nullptr;
        std::size_t _nextRecord = 0;
        std::size_t _droppedRecords = 0;
        Queue _dirty;
    };

}

#endif
