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

    template <typename Record_, int NumRecords_>
    using RecordArrayImpl = std::array<Record_, NumRecords_>;

    template <typename Record_, int NumRecords_>
    using RecordArrayQueueImpl = Queue::Queue<std::array<Record_, NumRecords_> >;

    template <typename Record_, int NumRecords_>
    using RecordArrayNodeImpl = typename RecordArrayQueueImpl<Record_, NumRecords_>::Node;

    template <typename Record_, int MaxBytes_>
    constexpr int minNumRecords()
    {
        auto emptyNodeSize = sizeof(Queue::Queue<std::array<Record_, 0> >);
        return (MaxBytes_ - emptyNodeSize) / sizeof(Record_);
    };

    template<int N_, int M_>
    struct LessThanOrEqual
    {
        enum {Value = N_ <= M_};
    };

    template <typename Record_, int MaxBytes_, int NumRecords_ = 0, int BytesLeft_ = MaxBytes_, // = minNumRecords<Record_, MaxBytes_>(),
              typename std::enable_if<LessThanOrEqual<BytesLeft_, 0>::Value>::type* = nullptr>
    constexpr int maxNumRecords()
    {
        return NumRecords_;
    }

    template<int N_, int M_>
    struct LessThan
    {
        enum {Value = N_ < M_};
    };

    template <typename Record_, int MaxBytes_, int NumRecords_ = 0, int BytesLeft_ = MaxBytes_, // = minNumRecords<Record_, MaxBytes_>(),
              typename std::enable_if<LessThan<0, BytesLeft_>::Value>::type* = nullptr>
    constexpr int maxNumRecords()
    {
        return maxNumRecords<Record_, MaxBytes_, NumRecords_ + 1, MaxBytes_ - sizeof(RecordArrayNodeImpl<Record_, NumRecords_ + 1>)>();
    }

    template <typename Record_, int MaxBytes_>
    struct RecordArray
    {
        static constexpr int NumRecords = maxNumRecords<Record_, MaxBytes_>();
        using Array = RecordArrayImpl<Record_, NumRecords>;
        using Queue = RecordArrayQueueImpl<Record_, NumRecords>;
        using Node = RecordArrayNodeImpl<Record_, NumRecords>;
    };

    template <typename Record_>
    struct ExtractorBase : RecordExtractor
    {
        using RecordType = Record_;
        using RecordArrayNode = typename Internal::RecordArray<Record_, Arena::DataSize>::Node;
      protected:
        void doStreamDirtyRecords(std::ostream& out_, RecordArrayNode* recordNode_)
        {
            // while (recordNode_) {
            //     DLOG("Streaming " << recordNode_);
            //     PROFILER_ASSERT(recordNode_->value.dirty());
            //     out_ << recordNode_->value;
            //     recordNode_ = recordNode_->getNext();
            // }
        }
    };

}

    template <typename Record_>
    struct SimpleExtractor : Internal::ExtractorBase<Record_>
    {
        using Base = Internal::ExtractorBase<Record_>;
        using RecordType = Record_;
        using RecordArrayNode = typename Internal::RecordArray<Record_, Arena::DataSize>::Node;
        SimpleExtractor(RecordArrayNode* const records_)
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
        RecordArrayNode* const _records;
    };

    template <typename Record_>
    struct RecordManager : Internal::ExtractorBase<Record_>
    {
        using Base = Internal::ExtractorBase<Record_>;
        using RecordType = Record_;
        using This = RecordManager<RecordType>;
        using RecordArrayTypes = Internal::RecordArray<Record_, Arena::DataSize>;
        // using Array = typename RecordArrayTypes::Array;
        using Node = typename RecordArrayTypes::Node;
        using Queue = typename RecordArrayTypes::Queue;

        RecordManager(Arena& arena_)
          : _arena(arena_),
            _dirty(arena_.basePtr())
        { }
        RecordManager(const This&) = delete;

        RecordType* getRecord()
        {
            if (!_current || _current->value.size() <= _nextRecord)
            {
                _current = _arena.acquire<Node>();
                _nextRecord = 0;
                DLOG("Arena acquire, block: " << _current);
            }
            if (!_current)
            {
                ++_droppedRecords;
                return nullptr;
            }
            return &_current->value[_nextRecord++];
        }

        void retireRecordArrayNode(Node& node_)
        {
            // PROFILER_ASSERT(node_.value.dirty());
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
        Node* _current = nullptr;
        std::size_t _nextRecord = 0;
        std::size_t _droppedRecords = 0;
        Queue _dirty;
    };

}
}

#endif
