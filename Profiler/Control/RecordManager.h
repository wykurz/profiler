#ifndef _PROFILER_CONTROL_RECORDMANAGER_H
#define _PROFILER_CONTROL_RECORDMANAGER_H

#include <Profiler/Control/Arena.h>
#include <Profiler/Exception/Exception.h>
#include <Profiler/Log/Log.h>
#include <Profiler/Queue/Queue.h>
#include <Profiler/Record/Record.h>
#include <memory>
#include <ostream>
#include <vector>

namespace Profiler {
namespace Control {

struct RecordExtractor {
  virtual ~RecordExtractor() = default;
  virtual void streamDirtyRecords(std::ostream &out_) = 0;
  virtual std::unique_ptr<RecordExtractor> moveToFinalExtractor() = 0;
};

namespace Internal {

template <typename Record_, int NumRecords_>
using RecordArrayImpl = std::array<Record_, NumRecords_>;

template <typename Record_, int NumRecords_>
using RecordArrayQueueImpl = Queue::Queue<std::array<Record_, NumRecords_>>;

template <typename Record_, int NumRecords_>
using RecordArrayNodeImpl =
    typename RecordArrayQueueImpl<Record_, NumRecords_>::Node;

template <typename Record_, int NumRecords_> constexpr int currentSize() {
  return sizeof(RecordArrayNodeImpl<Record_, NumRecords_>);
};

template <typename Record_, int Bytes_> constexpr int minNumRecords() {
  return (Bytes_ - currentSize<Record_, 1>()) / sizeof(Record_);
}

template <typename Record_, int Bytes_, int NumRecords_>
constexpr bool feasibleNumRecords() {
  return currentSize<Record_, NumRecords_>() < Bytes_;
}

template <typename Record_, int Bytes_,
          int NumRecords_ = minNumRecords<Record_, Bytes_>(),
          bool feasible =
              feasibleNumRecords<Record_, Bytes_, NumRecords_ + 1>()>
struct FindNumRecords;

template <typename Record_, int Bytes_, int NumRecords_>
struct FindNumRecords<Record_, Bytes_, NumRecords_, false> {
  static_assert(feasibleNumRecords<Record_, Bytes_, NumRecords_>(),
                "Current number of records we found should be feasible.");
#ifndef _MSC_VER
  // Visual Studio 15 (2017) doesn't compile this assertion for some reason.
  static_assert(!feasibleNumRecords<Record_, Bytes_, NumRecords_ + 1>(),
                "We shouldn't be able to increase # records any further.");
#endif
  enum { Value = NumRecords_ };
};

template <typename Record_, int Bytes_, int NumRecords_>
struct FindNumRecords<Record_, Bytes_, NumRecords_, true> {
  enum { Value = FindNumRecords<Record_, Bytes_, NumRecords_ + 1>::Value };
};

template <typename Record_, int MaxBytes_> struct RecordArray {
  static constexpr int NumRecords = FindNumRecords<Record_, MaxBytes_>::Value;
  static_assert(10 < NumRecords, "Too few records stored in an Array::Block");
  using Array = RecordArrayImpl<Record_, NumRecords>;
  using Queue = RecordArrayQueueImpl<Record_, NumRecords>;
  using Node = RecordArrayNodeImpl<Record_, NumRecords>;
};
} // namespace Internal

template <typename Record_> struct SimpleExtractor : RecordExtractor {
  using RecordType = Record_;
  using RecordArrayNode =
      typename Internal::RecordArray<Record_, Arena::DataSize>::Node;
  SimpleExtractor(Arena &arena_, RecordArrayNode *const records_, int numDirty_)
      : _arena(arena_), _records(records_), _numDirty(numDirty_) {}
  std::unique_ptr<RecordExtractor> moveToFinalExtractor() override {
    throw Exception::LogicError("Attempting to finalize SimpleExtractor");
  }
  void streamDirtyRecords(std::ostream &out_) override {
    while (_records) {
      auto &recordArray = _records->value;
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
  Arena &_arena;
  RecordArrayNode *_records;
  const std::size_t _numDirty;
};

template <typename Record_> struct RecordManager : RecordExtractor {
  using RecordType = Record_;
  using This = RecordManager<RecordType>;
  using RecordArrayTypes = Internal::RecordArray<Record_, Arena::DataSize>;
  // using Array = typename RecordArrayTypes::Array;
  using Node = typename RecordArrayTypes::Node;
  using Queue = typename RecordArrayTypes::Queue;

  explicit RecordManager(Arena &arena_)
      : _arena(arena_), _dirty(arena_.basePtr()) {}
  RecordManager(const This &) = delete;

  RecordType *getRecord() {
    if (!_current || _current->value.size() <= _nextRecord) {
      if (_current)
        _dirty.push(_current);
      _current = _arena.acquire<Node>();
      _nextRecord = 0;
      DLOG("Arena acquire, block: " << _current);
    }
    if (!_current) {
      ++_droppedRecords;
      return nullptr;
    }
    return &_current->value[_nextRecord++];
  }

  void streamDirtyRecords(std::ostream &out_) override {
    auto records = extractDirtyRecords();
    while (records) {
      auto &recordArray = records->value;
      for (int i = 0; i < recordArray.size(); ++i) {
        DLOG("Streaming " << recordArray[i]);
        out_ << recordArray[i];
      }
      auto next = records->getNext();
      _arena.release(records);
      records = next;
    }
  }

  std::unique_ptr<RecordExtractor> moveToFinalExtractor() override {
    if (_current)
      _dirty.push(_current);
    return std::make_unique<SimpleExtractor<RecordType>>(
        _arena, extractDirtyRecords(), _nextRecord);
  }

private:
  Node *extractDirtyRecords() {
    auto node = _dirty.extract();
    DLOG("extractDirtyRecords, node " << node);
    return node;
  }

  Arena &_arena;
  // TODO(mateusz): add padding
  Node *_current = nullptr;
  std::size_t _nextRecord = 0;
  std::size_t _droppedRecords = 0;
  Queue _dirty;
};
} // namespace Control
} // namespace Profiler

#endif
