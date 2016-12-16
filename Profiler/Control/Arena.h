#ifndef CONTROL_ARENA_H
#define CONTROL_ARENA_H

#include <Profiler/Algorithm/FreeMap.h>
#include <Profiler/Exception/Exception.h>
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

        Arena(std::size_t bufferSize_)
          : _nblocks(bufferSize_ / sizeof(BlockHolder)),
            _buffer(_nblocks * sizeof(BlockHolder))
        { }
        Arena(Arena&&) = delete;
        Arena(const Arena&) = delete;

        template <typename T_>
        Block<T_>* acquire()
        {
            static_assert(alignof(BlockHolder) % alignof(Block<T_>) == 0,
                          "Block<T> alignement not congruent to BlockHolder alignment.");
            int freeIdx = _freeMap.firstFree();
            if (freeIdx < 0) return nullptr;
            _freeMap.set(freeIdx, false);
            return reinterpret_cast<Block<T_>*>(getHolder(freeIdx));
        }

        template <typename T_>
        void release(Block<T_>* block_)
        {
            // Technically we don't need to require a typed pointer and could use void*, but we reserve the right to
            // allow potential future optimizations.
            PROFILER_ASSERT(block_);
            PROFILER_ASSERT(!_freeMap.isFree(getIndex(block_)));
            _freeMap.set(getIndex(block_), true);
        }

        /**
         * Base pointer for a given type T. All elements in a Block<T> will have positive offset from base pointer.
         *
         * Base pointer is guaranteed to never change during the lifetime of the arena.
         */
        template <typename T_>
        T_* basePtr()
        {
            PROFILER_ASSERT(reinterpret_cast<std::uintptr_t>(_buffer.data()) % alignof(T_) == 0);
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

        BlockHolder* getHolder(int index_)
        {
            return static_cast<BlockHolder*>(_buffer.data()) + index_;
        }

        int getIndex(void* ptr_) const
        {
            const auto offset = reinterpret_cast<std::intptr_t>(ptr_) - reinterpret_cast<std::intptr_t>(_buffer.data());
            PROFILER_ASSERT(0 <= offset && offset % sizeof(BlockHolder) == 0);
            return offset / sizeof(BlockHolder);
        }

        const std::size_t _nblocks;
        Algorithm::FreeMap _freeMap{_nblocks};
        Buffer _buffer;
        void* _next = _buffer.data();
        std::size_t _bytesLeft = _buffer.size();
    };

}
}

#endif
