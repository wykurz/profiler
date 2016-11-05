#include <Control/Manager.h>
#include <Log/Log.h>
#include <array>
#include <atomic>

namespace Profiler { namespace Control
{

    Manager::~Manager()
    {
        stopWriter();
        _writerThread.join();
    }

    // TODO: How should we deal with infinite # of threads
    Arena& Manager::addThread(Thread& thread_)
    {
        int count = MaxThreads;
        while (0 < count--) {
            auto& holder = _threadArray[_currentThread++];
            auto lk = holder.lock();
            if (holder.thread) continue;
            holder.thread = &thread_;
            return _arena;
        }
        ++_droppedThreads;
        return _empty;
    }

    void Manager::stopWriter()
    {
        _writer.stop();
    }

    Manager& getManager()
    {
        static Manager manager;
        return manager;
    }

}
}
