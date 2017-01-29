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

    template <typename Record_, int NumRecords_>
    constexpr int currentSize()
    {
        return sizeof(RecordArrayNodeImpl<Record_, NumRecords_>);
    };

    template <typename Record_, int NumRecords_, int MaxBytes_>
    constexpr int bytesLeft()
    {
        return MaxBytes_ - currentSize<Record_, NumRecords_>();
    };

    template<int N_, int M_>
    struct LessThan
    {
        enum {Value = N_ < M_};
    };

    template <typename Record_, int MaxBytes_, int NumRecords_ = minNumRecords<Record_, MaxBytes_>(),
              typename std::enable_if<LessThan<bytesLeft<Record_, NumRecords_, MaxBytes_>(), 0>::Value>::type* = nullptr>
    constexpr int maxNumRecords()
    {
        return NumRecords_ - 1;
    }

    template<int N_, int M_>
    struct LessThanOrEqual
    {
        enum {Value = N_ <= M_};
    };

    template <typename Record_, int MaxBytes_, int NumRecords_ = minNumRecords<Record_, MaxBytes_>(),
              typename std::enable_if<LessThanOrEqual<0, bytesLeft<Record_, NumRecords_, MaxBytes_>()>::Value>::type* = nullptr>
    constexpr int maxNumRecords()
    {
        return maxNumRecords<Record_, MaxBytes_, NumRecords_ + 1>();
    }

    template <typename Record_, int MaxBytes_>
    struct RecordArray
    {
        static constexpr int NumRecords = maxNumRecords<Record_, MaxBytes_>();
        static_assert(10 < NumRecords, "Too few records stored in an Array::Block");
        using Array = RecordArrayImpl<Record_, NumRecords>;
        using Queue = RecordArrayQueueImpl<Record_, NumRecords>;
        using Node = RecordArrayNodeImpl<Record_, NumRecords>;
    };

}

    template <typename Record_>
    struct SimpleExtractor : RecordExtractor
    {
        using RecordType = Record_;
        using RecordArrayNode = typename Internal::RecordArray<Record_, Arena::DataSize>::Node;
        SimpleExtractor(Arena& arena_, RecordArrayNode* const records_, int numDirty_)
          : _arena(arena_), _records(records_), _numDirty(numDirty_)
        { }
        virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() override
        {
            throw Exception::LogicError("Attempting to finalize SimpleExtractor");
        }
        virtual void streamDirtyRecords(std::ostream& out_) override
        {
            while (_records) {
                auto& recordArray = _records->value;
                auto size = recordArray.size();
                if (!_records->getNext()) {
                    PROFILER_ASSERT(_numDirty <= size);
                    size = _numDirty;
                }
                for (int i = 0; i < size; ++i) {
                    DLOG("Streaming " << recordArray[i]);
                    out_ << recordArray[i];
                }
                auto next = _records->getNext();
                _arena.release(_records);
                _records = next;
            }
        }
      private:
        Arena& _arena;
        RecordArrayNode* _records;
        const std::size_t _numDirty;
    };

    template <typename Record_>
    struct RecordManager : RecordExtractor
    {
        using RecordType = Record_;
        using This = RecordManager<RecordType>;
        using RecordArrayTypes = Internal::RecordArray<Record_, Arena::DataSize>;
        // using Array = typename RecordArrayTypes::Array;
        using Node = typename RecordArrayTypes::Node;
        using Queue = typename RecordArrayTypes::Queue;

        RecordManager(Arena& arena_)
          : _arena(arena_), _dirty(arena_.basePtr())
        { }
        RecordManager(const This&) = delete;

        RecordType* getRecord()
        {
            if (!_current || _current->value.size() <= _nextRecord)
            {
                if (_current) _dirty.push(_current);
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

        virtual void streamDirtyRecords(std::ostream& out_) override
        {
            auto records = extractDirtyRecords();
            while (records) {
                auto& recordArray = records->value;
                for (int i = 0; i < recordArray.size(); ++i) {
                    DLOG("Streaming " << recordArray[i]);
                    out_ << recordArray[i];
                }
                auto next = records->getNext();
                _arena.release(records);
                records = next;
            }
        }

        virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() override
        {
            if (_current) _dirty.push(_current);
            return std::make_unique<SimpleExtractor<RecordType> >(_arena, extractDirtyRecords(), _nextRecord);
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
