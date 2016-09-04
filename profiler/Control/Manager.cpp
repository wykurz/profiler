#include <Control/Manager.h>
#include <Log/Log.h>
#include <array>
#include <atomic>

namespace Control
{

    Manager::~Manager()
    {
        _writer.stop();
        _writerThread.join();
    }

    // TODO: How should we deal with infinite # of threads
    // TODO: Will we have multiple of those function?
    ThreadHolder* Manager::addThread(Thread& thread_)
    {
        ThreadHolder* res = nullptr;
        auto count = MaxSlotSearches;
        while (!res && 0 < count--) {
            auto& holder = _threadArray[_currentThread++];
            auto lk = holder.lock();
            if (holder.thread) continue;
            holder.thread = &thread_;
            res = &holder;
        }
        if (!res) ++_droppedThreads;
        return res;
    }

    Manager::RecordStorageType& Manager::getRecordStorage()
    {
        return _recordStorage;
    }

    Manager& getManager()
    {
        static Manager manager;
        return manager;
    }

}
