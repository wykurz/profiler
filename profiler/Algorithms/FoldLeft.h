#ifndef ALGORITHMS_FOLDLEFT_H
#define ALGORITHMS_FOLDLEFT_H

#include <algorithm>

namespace Profiler { namespace Algorithms
{

    template <typename Container_, typename Type_, typename Func_>
    Type_ foldLeft(const Container_& container_, Type_ initial_, Func_ func_)
    {
        return std::accumulate(std::begin(container_), std::end(container_), initial_, func_);
    }

}
}

#endif
