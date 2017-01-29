#ifndef LOG_LOG_H
#define LOG_LOG_H

#include <iostream>
#include <mutex>

namespace Profiler { namespace Log
{

    std::mutex& logMutex();

}
}

#ifdef DEBUG_LOG
#define DLOG(x) do { \
        std::unique_lock<std::mutex> lk(Profiler::Log::logMutex());                                      \
        std::cerr << __FILE__ << "(" << __LINE__ << "): " << __PRETTY_FUNCTION__ << " :: " << x << "\n"; \
    } while (false);
#else
#define DLOG(x) do { } while (false);
#endif

#endif
