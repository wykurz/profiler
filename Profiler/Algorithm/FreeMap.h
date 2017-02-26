#ifndef ALGORITHMS_BITMASK_H
#define ALGORITHMS_BITMASK_H

#include <Profiler/Exception/Exception.h>
#include <Profiler/Log/Log.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <limits>
#include <string>
#include <vector>

namespace Profiler {
namespace Algorithm {

struct FreeMap {
  using ChunkType = std::uint16_t;
  static constexpr std::size_t Chunks = 4;
  using BitmaskType = std::uint64_t;
  static constexpr std::size_t BitmaskSize = sizeof(BitmaskType) * 8;
  static_assert(sizeof(BitmaskType) == sizeof(ChunkType) * Chunks,
                "Bitmask must be a sum of chunks!");
  static constexpr std::size_t MaxSubtreeSize =
      (std::size_t(1) << (sizeof(ChunkType) * 8)) - 1;
  static constexpr std::size_t MaxSize = Chunks * MaxSubtreeSize;

  FreeMap(std::size_t size_) {
    PROFILER_ASSERT(size_ <= MaxSize);
    // TODO: Not the most efficient way of initializing self.
    for (unsigned index = 0; index < size_; ++index)
      setFree(index);
    DLOG("Buckets: " << _buckets[0][0].load() << ", " << _buckets[0][1].load()
                     << ", " << _buckets[0][2].load() << ", "
                     << _buckets[0][3].load());
  }

  int getFree() {
    int index = 0;
    for (int level = 0; level < NumLevels; ++level) {
      PROFILER_ASSERT(_indexOfLevel[level] <= index);
      auto &bucket = _buckets[index];
      int ifound = -1;
      for (int ichunk = 0; ichunk < Chunks; ++ichunk) {
        ChunkType chunk;
        do {
          chunk = bucket[ichunk].load(std::memory_order_acquire);
          if (0 == chunk)
            break;
          // We found a chunk with > 0 slots available.
          if (bucket[ichunk].compare_exchange_strong(
                  chunk, chunk - 1, std::memory_order_release,
                  std::memory_order_relaxed)) {
            ifound = ichunk;
            break;
          }
          // We weren't able to swap the chunk with the updated reduced value.
        } while (0 < chunk);
        if (-1 < ifound)
          break;
      }
      if (ifound < 0 && 0 == level) {
        DLOG("Failed to find an empty slot");
        return -1;
      }
      PROFILER_ASSERT(-1 < ifound);
      // Update index to the index of it's ifound-th child
      index = getChildIndex(index, ifound);
    }
    // For usability, we want to return the last level index rather than the
    // node index in the whole bucket tree.
    index -= NumBuckets;
    PROFILER_ASSERT(0 <= index);
    PROFILER_ASSERT(index <= NumBitmasks);
    auto &bitmask = _bitmasks[index];
    auto unsetOneBit = [](BitmaskType bitmask_) {
      PROFILER_ASSERT(bitmask_);
      return (bitmask_ - 1) & bitmask_;
    };
    BitmaskType newBitmask = bitmask.load(std::memory_order_acquire);
    while (!bitmask.compare_exchange_weak(newBitmask, unsetOneBit(newBitmask),
                                          std::memory_order_release,
                                          std::memory_order_relaxed))
      PROFILER_ASSERT(0 < newBitmask);
    BitmaskType newBit = newBitmask & ~unsetOneBit(newBitmask);
    int bitIndex = __builtin_ffsl(newBit) - 1;
    PROFILER_ASSERT(0 <= bitIndex);
    const int trueIndex = index * sizeof(BitmaskType) * 8 + bitIndex;
    return trueIndex - trueIndex / (MaxSubtreeSize + 1);
  }

  void setFree(unsigned index_) {
    const unsigned trueIndex = index_ + index_ / MaxSubtreeSize;
    auto setOneBit = [](int bitIndex_, BitmaskType bitmask_) {
      auto oneBit = BitmaskType(1) << bitIndex_;
      PROFILER_ASSERT(!(oneBit & bitmask_));
      return bitmask_ | oneBit;
    };
    const int ibitmask = trueIndex / BitmaskSize;
    PROFILER_ASSERT(ibitmask < NumBitmasks);
    auto &bitmask = _bitmasks[ibitmask];
    auto newBitmask = bitmask.load(std::memory_order_acquire);
    while (!bitmask.compare_exchange_weak(
        newBitmask, setOneBit(trueIndex % BitmaskSize, newBitmask),
        std::memory_order_release, std::memory_order_relaxed)) {
      PROFILER_ASSERT(newBitmask < std::numeric_limits<BitmaskType>::max());
    }
    int ibucket = ibitmask + NumBuckets;
    do {
      int iparent = getParentIndex(ibucket);
      PROFILER_ASSERT(iparent < NumBuckets);
      const int firstChild = getChildIndex(iparent, 0);
      const int ichunk = ibucket - firstChild;
      PROFILER_ASSERT(ichunk < Chunks);
      auto pbucket =
          _buckets[iparent][ichunk].fetch_add(1, std::memory_order_release);
      PROFILER_ASSERT(pbucket < std::numeric_limits<decltype(pbucket)>::max());
      ibucket = iparent;
    } while (ibucket > 0);
  }

  /**
   * Used for testing, the order in which free slots are acquired should not be
   * depended on.
   */
  bool isFree(unsigned index_) const {
    const unsigned trueIndex = index_ + index_ / MaxSubtreeSize;
    auto bitmaskIndex = trueIndex / (sizeof(BitmaskType) * 8);
    PROFILER_ASSERT(bitmaskIndex < NumBitmasks);
    auto bitmask = _bitmasks[bitmaskIndex].load(std::memory_order_acquire);
    return bitmask & (BitmaskType(1) << (trueIndex % BitmaskSize));
  }

private:
  /**
   * [2^16|2^16|2^16|2^16]
   *    |    |    |    |
   *    |    |    |  [2^14|...]
   *    |    |    |    | ...
   *    |    |    |    *
   *    |    |    |
   *    |    |  [2^14|...]
   *    |    |    | ...
   *    |    |    *
   *    |    |
   *    |  [2^14|...]
   *    |    | ...
   *    |    *
   *    |
   * [2^14|2^14|2^14|2^14]
   *    |    |    |    |
   *    |    |    |  [2^12|...]
   *    |    |    |    | ...
   *    |    |    |    *
   *    |    |    |
   *    |    |  [2^12|...]
   *    |    |    | ...
   *    |    |    *
   *    |    |
   *    |  [2^12|...]
   *    |    | ...
   *    .    *
   *    .
   *    .
   *    |
   * [2^4|2^4|2^4|2^4] <--- Last level could be represented as a bitmask since
   * 2^4 == 16.
   * ^^^^^^^^^^^^^^^^^
   * [      2^64     ] <--- In fact, we can represent it as a 64-bit (4x 16)
   * value.
   *
   * Number of levels    - len([16, 14, ..., 8, 6]) = 6 (+ bit masks)
   * Number of buckets   - 1 + 4 + 4^2 + ... + 4^5 = (4^6 - 1) / (4 - 1)
   * Number of bit-masks - 4^6
   *
   * Note that the most top-level chunk can represent only 2^16 - 1 free slots.
   * This means that the last bitmask
   * in each subtree starting from the first-level child may hold one less
   * element than all the other bitmasks.
   * This is accounted for in the algorithm and is made transparent to the user
   * by index arithmetic.
   */
  static constexpr std::size_t NumLevels = 6;
  static constexpr std::size_t NumBuckets =
      ((1 << (2 * NumLevels)) - 1) / (4 - 1);
  static constexpr std::size_t NumBitmasks = 1 << (2 * NumLevels);

  static int getChildIndex(int parentIndex_, int child_) {
    return parentIndex_ * Chunks + 1 + child_;
  }

  static std::size_t getParentIndex(std::size_t childIndex_) {
    return (childIndex_ - 1) / Chunks;
  }

  std::array<std::array<std::atomic<ChunkType>, Chunks>, NumBuckets> _buckets{};
  std::array<std::atomic<BitmaskType>, NumBitmasks> _bitmasks{};

  // TODO: use template to generate
  std::array<std::size_t, NumLevels> _indexOfLevel{
      {0, 1, 1 << 2, 1 << 4, 1 << 6, 1 << 8}};
};
}
}

#endif
