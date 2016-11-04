#ifndef CONTROL_ARENA_H
#define CONTROL_ARENA_H

#include <array>
#include <cassert>
#include <memory>
#include <sstream>
#include <vector>

namespace Control
{

    /**
     * A simplified allocator.
     */
    struct Arena
    {
        constexpr static std::size_t BlockSize = 1024;

        Arena(std::size_t bufferSize_)
          : _buffer(bufferSize_)
        { }
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
                auto res = reinterpret_cast<TBlock*>(_next);
                _next = res + 1;
                _bytesLeft -= sizeof(TBlock);
                return res;
            }
            return nullptr;
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
            void* base = const_cast<char*>(_buffer.data());
            std::size_t bytes = _bytesLeft;
            if (std::align(alignof(TBlock), sizeof(TBlock), base, bytes))
            {
                return &(*reinterpret_cast<TBlock*>(base))[0];
            }
            std::stringstream ss;
            ss << "Failed to obtain base pointer for type " << typeid(T_).name() << ", " << sizeof(TBlock);
            throw std::runtime_error(ss.str());
        }

      private:
        // TODO: Need to make this dynamic
        std::vector<char> _buffer;
        void* _next = _buffer.data();
        std::size_t _bytesLeft = _buffer.size();
    };

}

#endif
