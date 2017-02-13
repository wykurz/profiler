#ifndef ALGORITHMS_BITMASK_H
#define ALGORITHMS_BITMASK_H

#include <Profiler/Exception/Exception.h>
#include <Profiler/Log/Log.h>
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

namespace Profiler { namespace Algorithm
{

    struct FreeMap
    {
        static constexpr std::size_t MaxSize = 1 << 10;

        FreeMap(std::size_t size_)
          : data((size_ + 7) / 8)
        {
            if (size_ && (0 != size_ % 8)) {
                int reminder = size_ % 8;
                data[data.size() -1] = ((1 << (8 - reminder)) - 1) << reminder;
            }
        }

        int getFree()
        {
            auto f = [](char c) {
                for (int i = 0; i < 8; ++i) if (!(c & (1 << i))) return i;
                PROFILER_ASSERT(false);
            };
            for (int i = 0; i < data.size(); ++i) {
                if (-1 != data[i]) {
                    int index = i * 8 + f(data[i]);
                    set(index, false);
                    return index;
                }
            }
            return -1;
        }

        void setFree(std::size_t index_)
        {
            PROFILER_ASSERT(!isFree(index_));
            set(index_, true);
        }

        /**
         * Used for testing, the order in which free slots are acquired should not be depended on.
         */
        bool isFree(std::size_t index_) const
        {
            PROFILER_ASSERT(index_ / 8 < data.size());
            return !(data[index_ / 8] & (1 << (index_ % 8)));
        }

        /**
         * Used for testing.
         */
        std::string str() const
        {
            std::string s(data.size() * 8, '0');
            int i = 0;
            for (auto c : data) for (int j = 0; j < 8; ++i, ++j) if (c & (1 << j)) s[i] = '1';
            return s;
        }

      private:
        void set(std::size_t index_, bool free_)
        {
            PROFILER_ASSERT(index_ / 8 < data.size());
            if (free_) data[index_ / 8] &= ~(1 << (index_ % 8));
            else data[index_ / 8] |= (1 << (index_ % 8));
        }

        std::vector<char> data;
    };

}
}

#endif
