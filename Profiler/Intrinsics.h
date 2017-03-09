#ifndef INTRINSICS_H
#define INTRINSICS_H

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace Profiler {
namespace Intrinsics {

    inline unsigned long long rdtsc()
    {
      return __rdtsc();
    }

    inline int ffsl(unsigned long mask_)
    {
#ifdef _MSC_VER
      unsigned long res;
      if (0 == _BitScanReverse64(&res, mask_)) return 0;
      return res;
#else
      return __builtin_ffsl(mask_);
#endif
    }

} // namespace Intrinsics
} // namespace Profiler

#endif
