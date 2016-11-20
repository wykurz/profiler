#include <Log/Log.h>
#include <mutex>

namespace Profiler { namespace Log
{

    std::mutex& logMutex()
    {
        static std::mutex mutex;
        return mutex;
    }

}
}
