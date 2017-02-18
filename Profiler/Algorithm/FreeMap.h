#ifndef ALGORITHMS_BITMASK_H
#define ALGORITHMS_BITMASK_H

#include <Profiler/Exception/Exception.h>
#include <Profiler/Log/Log.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <string>
#include <vector>

namespace Profiler { namespace Algorithm
{

    struct FreeMap
    {
        using ChunkType = std::uint16_t;
        static constexpr std::size_t Chunks = 4;
        using BitmaskType = std::uint64_t;
        static_assert(sizeof(BitmaskType) == sizeof(ChunkType) * Chunks, "Bitmask must be a sum of chunks!");
        static constexpr std::size_t MaxSize = Chunks * (1 << (sizeof(ChunkType) * 8)) - 1;

        int getFree()
        {
            int index = 0;
            for (int level = 0; level < NumLevels - 1; ++level) {
                PROFILER_ASSERT(_indexOfLevel[level] <= index);
                auto& bucket = _buckets[index];
                int ifound = -1;
                for (int ichunk = 0; ichunk < Chunks; ++ichunk) {
                    ChunkType chunk;
                    do {
                        chunk = bucket[ichunk].load(std::memory_order_acquire);
                        if (0 == chunk) break;
                        // We found a chunk with > 0 slots available.
                        if (bucket[ichunk].compare_exchange_strong(
                                chunk, chunk - 1, std::memory_order_release, std::memory_order_relaxed)) {
                            ifound = ichunk;
                            break;
                        }
                        // We weren't able to swap the chunk with the updated reduced value.
                    } while (0 < chunk);
                    if (-1 < ifound) break;
                }
                if (ifound < 0 && 0 == level) return -1;
                PROFILER_ASSERT(-1 < ifound);
                // Update index to the index of it's ifound-th child
                index = index * Chunks + 1 + ifound;
            }
            index -= _indexOfLevel[NumLevels - 1];
            PROFILER_ASSERT(0 <= index);
            PROFILER_ASSERT(index <= NumBitmasks);
            auto& bitmask = _bitmasks[index];
            auto setOneBit = [](BitmaskType bitmask_) {
                return (bitmask_ + 1) | bitmask_;
            };
            BitmaskType nbitmask = bitmask.load(std::memory_order_acquire);
            BitmaskType pbitmask = nbitmask;
            while (!bitmask.compare_exchange_strong(
                       nbitmask, setOneBit(nbitmask), std::memory_order_release, std::memory_order_relaxed)) {
                PROFILER_ASSERT(nbitmask < std::numeric_limits<BitmaskType>::max());
                pbitmask = nbitmask;
            }
            BitmaskType newBit = nbitmask & ~pbitmask;
            int bitIndex = __builtin_ffsl(newBit) - 1;
            PROFILER_ASSERT(0 <= bitIndex);
            return index * sizeof(BitmaskType) * 8 + bitIndex;
        }

        void setFree(std::size_t index_)
        {
        }

        /**
         * Used for testing, the order in which free slots are acquired should not be depended on.
         */
        bool isFree(std::size_t index_) const
        {
            auto bucketIdx = index_ / 64;
            PROFILER_ASSERT(_indexOfLevel[NumLevels - 1] + bucketIdx < NumBuckets);
            auto bucket = _buckets[_indexOfLevel[NumLevels - 1] + bucketIdx].load(std::memory_order_acquire);
            auto bucketOffset = index_ % 64;
            return bucket & (decltype(bucket)(1) << bucketOffset);
        }

        /**
         * Used for testing.
         */
        std::string str() const
        {
            std::string s(_buckets.size() * 8, '0');
            int i = 0;
            for (auto& bucket : _buckets) {
                auto b = bucket.load();
                for (int j = 0; j < sizeof(b) * 8; ++i, ++j) if (b & (1 << j)) s[i] = '1';
            }
            return s;
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
         * [2^4|2^4|2^4|2^4] <--- Last level is represented as bitmask since 2^4 == 16.
         * ^^^^^^^^^^^^^^^^^
         * [      2^64     ] <--- In fact, we can represent it as a 64-bit mask.
         *
         * Number of levels    - len([16, 14, ..., 8, 6]) = 6 (+ bit masks)
         * Number of buckets   - 1 + 4 + 4^2 + ... + 4^5 = (4^6 - 1) / (4 - 1)
         * Number of bit-masks - 4^6
         *
         */
        static constexpr std::size_t NumLevels = 6;
        static constexpr std::size_t NumBuckets = ((1 << (2 * NumLevels)) - 1) / (4 - 1);
        static constexpr std::size_t NumBitmasks = 1 << (2 * NumLevels);

        std::array<std::array<std::atomic<ChunkType>, Chunks>, NumBuckets> _buckets{};
        std::array<std::atomic<BitmaskType>, NumBitmasks> _bitmasks{};

        // TODO: use template to generate
        std::array<std::size_t, NumLevels> _indexOfLevel{{0, 1, 1 << 2, 1 << 4, 1 << 6, 1 << 8}};
    };

}
}

#endif
