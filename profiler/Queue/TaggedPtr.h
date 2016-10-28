#ifndef QUEUE_TAGGEDPTR_H
#define QUEUE_TAGGEDPTR_H

#include <cstdint>

namespace Queue
{

    // TODO: run clang undef. behavior sanitizer
    template <typename T_>
    struct alignas(sizeof(std::intptr_t)) TaggedPtr
    {
        TaggedPtr() = default;
        TaggedPtr(const T_* base_, const T_* ptr_)
          : offset(getOffset(base_, ptr_)),
            tag(getTag(ptr_))
        {
            static_assert(sizeof(std::intptr_t) == sizeof(TaggedPtr<T_>),
                          "Tagged pointer doesn't match the expected size!");
            assert(base_);
            assert(!ptr_ || base_ <= ptr_);
        }
        TaggedPtr(const TaggedPtr&) = default;
        T_* get(T_* baseTagged_) const
        {
            return baseTagged_ + offset;
        }
        bool isNull() const
        {
            return -1 == offset;
        }
      private:
        static std::uint32_t getOffset(const T_* base_, const T_* ptr_)
        {
            if (!ptr_) return -1;
            return static_cast<std::uint32_t>(ptr_ - base_);
        }
        static std::uint32_t getTag(const T_* ptr_)
        {
            if (!ptr_) return 0;
            return ptr_->getTag();
        }
        const std::uint32_t offset = -1;
        const std::uint32_t tag = 0;
    };

}

#endif
