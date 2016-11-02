#ifndef CONTROL_ARENA_H
#define CONTROL_ARENA_H

#include <array>
#include <memory>

namespace Control
{

    /**
     * A simplified allocator.
     */
    struct Arena
    {
        constexpr static std::size_t BlockSize = 1024;

        Arena(const Arena&) = delete;
        Arena(Arena&&) = delete;

        template <typename T_>
        using Block = std::array<T_, BlockSize>;

        template <typename T_>
        Block<T_>* acquire()
        {
            using TBlock = Block<T_>;
            if (std::align(alignof(TBlock), sizeof(TBlock), _next, _bytesLeft))
            {
                // TODO: verify
                return reinterpret_cast<TBlock*>(_next);
            }
            throw std::exception();
        }

        template <typename T_>
        void release(Block<T_> block_)
        {
        }

        /**
         * Base pointer for a given type.
         *
         * Garanteed to never change during the lifetime of the arena.
         */
        template <typename T_>
        T_* basePtr() const
        {
            using TBlock = Block<T_>;
            void* tmpBase = _base;
            std::size_t tmpBytesLeft = _bytesLeft;
            if (std::align(alignof(TBlock), sizeof(TBlock), tmpBase, tmpBytesLeft))
            {
                return (*reinterpret_cast<TBlock*>(tmpBase))[0];
            }
            throw std::exception();
        }

      private:
        // TODO: Need to make this dynamic
        constexpr static std::size_t BufferSize = 100000;
        char _buffer[BufferSize];
        void* _base;
        void* _next = _buffer;
        std::size_t _bytesLeft = BufferSize;
    };

}

#endif
