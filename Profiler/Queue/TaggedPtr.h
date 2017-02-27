#ifndef _PROFILER_QUEUE_TAGGEDPTR_H
#define _PROFILER_QUEUE_TAGGEDPTR_H

#include <Profiler/Exception/Exception.h>

namespace Profiler {
namespace Queue {

// TODO(mateusz): run clang undef. behavior sanitizer
template <typename T_> struct alignas(alignof(std::intptr_t)) TaggedPtr {
  TaggedPtr() = default;
  TaggedPtr(const void *base_, const T_ *ptr_)
      : _offset(getOffset(base_, ptr_)), _tag(getTag(ptr_)) {
    static_assert(sizeof(std::intptr_t) == sizeof(TaggedPtr<T_>),
                  "Tagged pointer doesn't match the expected size!");
    PROFILER_ASSERT(base_);
    PROFILER_ASSERT(!ptr_ || base_ <= ptr_);
  }
  TaggedPtr(const TaggedPtr &) = default;
  T_ *get(void *baseTagged_) const {
    return reinterpret_cast<T_ *>(static_cast<char *>(baseTagged_) + _offset);
  }
  bool isNull() const { return -1 == _offset; }

private:
  static std::uint32_t getOffset(const void *base_, const void *ptr_) {
    if (ptr_ == nullptr)
      return -1;
    auto res = static_cast<std::uint32_t>(static_cast<const char *>(ptr_) -
                                          static_cast<const char *>(base_));
    PROFILER_ASSERT(static_cast<const char *>(base_) + res == ptr_);
    return res;
  }
  static std::uint32_t getTag(const T_ *ptr_) {
    if (!ptr_)
      return 0;
    return ptr_->getTag();
  }
  const std::uint32_t _offset = -1;
  const std::uint32_t _tag = 0;
};
} // namespace Queue
} // namespace Profiler

#endif
