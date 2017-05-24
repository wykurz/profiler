#ifndef _PROFILER_CONTROL_RECORDMANAGER_H
#define _PROFILER_CONTROL_RECORDMANAGER_H

#include <Profiler/Control/Arena.h>
#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <Profiler/Queue/Queue.h>
#include <memory>
#include <ostream>
#include <vector>

namespace Profiler {
namespace Control {

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

// TODO(mateusz): Convert into InputIterator:
// http://en.cppreference.com/w/cpp/concept/InputIterator
template <typename Record_> struct DirtyRecordsIter {
  using RecordType = Record_;
  using This = DirtyRecordsIter<RecordType>;
  using Node = typename Internal::RecordArray<Record_, Arena::DataSize>::Node;
  DirtyRecordsIter() = default;
  DirtyRecordsIter(Arena &arena_, Node *const records_, std::size_t lastNodeSize_)
      : _arena(&arena_), _records(records_), _lastNodeSize(lastNodeSize_) {}
  ~DirtyRecordsIter() {
    if (!_arena) return;
    while (_records) {
      auto next = _records->getNext();
      _arena->release(_records);
      _records = next;
    }
  }
  RecordType* next() {
    if (!_records) return nullptr;
    auto &recordArray = _records->value;
    auto size = recordArray.size();
    if (!_records->getNext()) {
      size = _lastNodeSize;
    }
    PROFILER_ASSERT(_nextRecord <= size);
    auto ans = recordArray[_nextRecord++];
    if (size <= _nextRecord) {
      auto _prev = _records;
      _records = _records->getNext();
      _arena->release(_prev);
      _nextRecord = 0;
    }
    return ans;
  }

 private:
  Arena *_arena = nullptr;
  Node *_records = nullptr;
  std::size_t _lastNodeSize = 0;
  std::size_t _nextRecord = 0;
};

template <typename Record_> struct RecordManager {
  using RecordType = Record_;
  using This = RecordManager<RecordType>;
  using RecordArrayTypes = Internal::RecordArray<Record_, Arena::DataSize>;
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
  DirtyRecordsIter<RecordType> getDirtyRecords() {
    return {_arena, _dirty.extract(), 0};
  }
  DirtyRecordsIter<RecordType> getFinalRecords() {
    if (_current)
      _dirty.push(_current);
    return {_arena, _dirty.extract(), _nextRecord};
  }

private:
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
