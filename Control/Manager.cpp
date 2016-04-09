#include <Control/Manager.h>
#include <array>
#include <atomic>

namespace Control
{

    struct Manager
    {
        static constexpr std::size_t MaxThreads = 1024;
        // TODO: How should we deal with infinite # of threads
        std::atomic<int> currentThread;
        std::array<Thread*, MaxThreads> threadBuffers;
    };

    Manager& getManager()
    {
        static Manager manager;
        return manager;
    }

    Thread::Thread()
    {
        auto& manager = getManager();
        auto id = manager.currentThread++;
        manager.threadBuffers[id] = this;
    }

    Thread& getThread()
    {
        thread_local static Thread thread;
        return thread;
    }

}
