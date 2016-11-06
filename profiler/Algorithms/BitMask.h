#ifndef ALGORITHMS_BITMASK_H
#define ALGORITHMS_BITMASK_H

#include <Exception/Exception.h>
#include <Log/Log.h>
#include <algorithm>
#include <cassert>

namespace Profiler { namespace Algorithms { namespace Test
{

    struct FreeMap
    {
        FreeMap(std::size_t size_)
          : data((size_ + 7) / 8)
        {
            if (size_ && (0 != size_ % 8)) {
                int reminder = size_ % 8;
                data[data.size() -1] = ((1 << (8 - reminder)) - 1) << reminder;
            }
            DLOG("FreeMap:" << str());
        }

        int firstFree() const
        {
            auto f = [](char c) {
                for (int i = 0; i < 8; ++i) if (!(c & (1 << i))) return i;
                assert(false);
                return -1;
            };
            for (int i = 0; i < data.size(); ++i) if (-1 != data[i]) return i * 8 + f(data[i]);
            return -1;
        }

        int lastFree() const
        {
            auto f = [](char c) {
                for (int i = 7; 0 <= i; --i) if (!(c & (1 << i))) return i;
                assert(false);
                return -1;
            };
            for (int i = data.size() - 1; 0 <= i; --i) if (-1 != data[i]) return i * 8 + f(data[i]);
            return -1;
        }

        void set(std::size_t index_, bool free_)
        {
            assert(index_ / 8 < data.size());
            if (free_) data[index_ / 8] &= ~(1 << (index_ % 8));
            else data[index_ / 8] |= (1 << (index_ % 8));
        }

        bool isFree(std::size_t index_)
        {
            assert(index_ / 8 < data.size());
            return !(data[index_ / 8] & (1 << (index_ % 8)));
        }

        std::string str() const
        {
            std::string s(data.size() * 8, '0');
            int i = 0;
            for (auto c : data) for (int j = 0; j < 8; ++i, ++j) if (c & (1 << j)) s[i] = '1';
            return s;
        }

      private:
        std::vector<char> data;
    };

}
}
}

#endif
