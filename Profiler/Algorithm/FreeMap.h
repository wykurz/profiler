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
        static constexpr std::size_t MaxSize = Chunks * (1 << (sizeof(ChunkType) * 8)) - 1;

        int getFree()
        {
            int index = 0;
            for (int level = 0; level < NumLevels; ++level) {
                auto& bucket = _buckets[index];
                int ifound = -1;
                for (int ichunk = 0; ichunk < Chunks; ++ichunk) {
                    auto chunk = bucket[ichunk].load(std::memory_order_acquire);
                    if (0 < chunk) {
                        if (bucket[ichunk].compare_exchange_strong(
                                chunk, chunk - 1, std::memory_order_release, std::memory_order_relaxed)) {
                            ifound = ichunk;
                            break;
                        }
                    }
                }
                if (ifound < 0) {
                    while (0 < level--) {
                        int parent = index / Chunks;
                        int base = parent * Chunks + 1;
                        PROFILER_ASSERT(index - base < Chunks);
                        _buckets[parent][index - base].fetch_add(1, std::memory_order_release);
                    }
                }
            }
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
         * [2^4|2^4|2^4|2^4] <--- last level is represented as bitmask since 2^4 == 16
         *
         * Number of levels - len([4, 6, ..., 14, 16]) = 7
         *
         * Number of elements - 1 + 4 + 4^2 + ... + 4^6 = (4^7 - 1) / (4 - 1)
         *
         */
        static constexpr std::size_t NumLevels = 7;
        static constexpr std::size_t NumBuckets = ((1 << (2 * NumLevels)) - 1) / (4 - 1);

        std::array<std::array<std::atomic<ChunkType>, Chunks>, NumBuckets> _buckets{};

        // TODO: use template to generate
        std::array<std::size_t, NumLevels> _indexOfLevel{{0, 1, 1 << 2, 1 << 4, 1 << 6, 1 << 8, 1 << 10}};
    };

}
}

#endif
