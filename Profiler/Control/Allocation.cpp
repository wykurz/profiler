#include <Profiler/Control/Allocation.h>

namespace Profiler { namespace Control
{

    std::atomic<Holder::Id> Allocation::_globalId{0};

}
}
