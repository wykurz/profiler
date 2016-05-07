#include <Control/Manager.h>
#include <array>
#include <atomic>

namespace Control
{

    // TODO: How should we deal with infinite # of threads
    // TODO: Will we have multiple of those function?
    void Manager::addThread(Thread& thread_)
    {
        bool found = false;
        auto count = MaxSlotSearches;
        while (!found && 0 < count--) {
            auto id = _currentThread++;
            std::unique_lock<std::mutex> lk(_threadArray[id].lock);
            if (_threadArray[id].thread) continue;
            _threadArray[id].thread = &thread_;
            found = true;
        }
        if (!found) ++_droppedThreads;
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
