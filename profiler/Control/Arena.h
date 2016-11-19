#ifndef CONTROL_ARENA_H
#define CONTROL_ARENA_H

#include <Algorithms/FreeMap.h>
#include <Exception/Exception.h>
#include <array>
#include <cstddef>
#include <memory>
#include <sstream>
#include <vector>

namespace Profiler { namespace Control
{

    /**
     * A simplified allocator.
     */
    struct Arena
    {
        constexpr static std::size_t BlockSize = 1024 * 32;
        template <typename T_> using Block = std::array<T_, BlockSize / sizeof(T_)>;
        using BlockHolder = std::aligned_storage<BlockSize, alignof(std::max_align_t)>::type;

        // TODO: Write tests to check we actually handle size 0
        Arena(std::size_t bufferSize_)
          : _nblocks(bufferSize_ / sizeof(BlockHolder)),
            _buffer(_nblocks * sizeof(BlockHolder))
        { }
        Arena(Arena&&) = delete;
        Arena(const Arena&) = delete;

        template <typename T_>
        Block<T_>* acquire()
        {
            using TBlock = Block<T_>;
            if (std::align(alignof(TBlock), sizeof(TBlock), _next, _bytesLeft))
            {
                auto res = reinterpret_cast<TBlock*>(_next);
                _next = res + 1;
                _bytesLeft -= sizeof(TBlock);
                return res;
            }
            return nullptr;
        }

        template <typename T_>
        void release(Block<T_>* block_)
        {
            PROFILER_ASSERT(block_);
            // TODO: ...
        }

        /**
         * Base pointer for a given type T. All elements in a Block<T> will have positive offset from base pointer.
         *
         * Base pointer is guaranteed to never change during the lifetime of the arena.
         */
        template <typename T_>
        T_* basePtr()
        {
            PROFILER_ASSERT(_buffer.data() % alignof(T_) == 0);
            return static_cast<T_*>(_buffer.data());
        }

      private:
        struct alignas(BlockHolder) Buffer
        {
            Buffer(std::size_t size_)
              : _buffer(size_)
            { }
            void* data()
            {
                return _buffer.data();
            }
            const void* data() const
            {
                return _buffer.data();
            }
            std::size_t size() const
            {
                return _buffer.size();
            }
          private:
            std::vector<char> _buffer;
        };

        const std::size_t _nblocks;
        Algorithms::FreeMap freeMap{_nblocks};
        Buffer _buffer;
        void* _next = _buffer.data();
        std::size_t _bytesLeft = _buffer.size();
    };

}
}

#endif
