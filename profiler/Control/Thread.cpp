#include <Control/Manager.h>
#include <Control/Thread.h>
#include <array>
#include <atomic>

namespace Profiler { namespace Control
{

    Thread::~Thread()
    {
        // TODO: Grab a slot-lock and deregister ourselves from Manager's thread buffer list
    }

    Thread& getThread()
    {
        // TODO: Setup everythin such that it's easy to avoid singletons on demand (e.g. in tests, but not only)
        thread_local static Thread thread(getManager());
        return thread;
    }

}
}
