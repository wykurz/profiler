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
        constexpr static std::size_t DataSize = 1024 * 32;
        using Block = std::aligned_storage<DataSize, alignof(std::max_align_t)>::type;

        Arena(std::size_t bufferSize_)
          : _nblocks(bufferSize_ / sizeof(Block)),
            _buffer(_nblocks * sizeof(Block))
        { }
        Arena(Arena&&) = delete;
        Arena(const Arena&) = delete;

        template <typename T_>
        T_* acquire()
        {
            static_assert(sizeof(T_) <= DataSize, "Blocks is to small to hold the requested object.");
            static_assert(alignof(Block) % alignof(T_) == 0, "Block alignement not congruent to requested type alignment.");
            int freeIdx = _freeMap.getFree();
            if (freeIdx < 0) return nullptr;
            return reinterpret_cast<T_*>(getHolder(freeIdx));
        }

        template <typename T_>
        void release(T_* block_)
        {
            // Technically we don't need to require a typed pointer and could use void*, but we reserve the right to
            // allow potential future optimizations.
            static_assert(sizeof(T_) <= DataSize, "Blocks is to small to hold the requested object.");
            static_assert(alignof(Block) % alignof(T_) == 0, "Block alignement not congruent to requested type alignment.");
            PROFILER_ASSERT(block_);
            PROFILER_ASSERT(!_freeMap.isFree(getIndex(block_)));
            _freeMap.setFree(getIndex(block_));
        }

        /**
         * Base pointer for a given type T. All Block-s in Arena will have a positive offset from the base pointer.
         *
         * Base pointer is guaranteed to never change during the lifetime of the arena.
         */
        char* basePtr()
        {
            return static_cast<char*>(_buffer.data());
        }

      private:
        struct alignas(Block) Buffer
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

        Block* getHolder(int index_)
        {
            return static_cast<Block*>(_buffer.data()) + index_;
        }

        int getIndex(void* ptr_) const
        {
            const auto offset = reinterpret_cast<std::intptr_t>(ptr_) - reinterpret_cast<std::intptr_t>(_buffer.data());
            PROFILER_ASSERT(0 <= offset && offset % sizeof(Block) == 0);
            return offset / sizeof(Block);
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
