#include <Control/Manager.h>
#include <array>
#include <atomic>

namespace Control
{

    Thread::Thread()
      : _recordManager(getManager().getNodeBase(), getManager().MaxRecords)
    {
        auto& manager = getManager();
        auto id = manager.currentThread++;
        manager.threadBuffers[id] = this;
    }

    Thread& getThread()
    {
        // TODO: Setup everythin such that it's easy to avoid singletons on demand (e.g. in tests, but not only)
        thread_local static Thread thread;
        return thread;
    }

    Manager& getManager()
    {
        static Manager manager;
        return manager;
    }

    template <>
    RecordManager<Record::Record>& Thread::getRecordManager<Record::Record>()
    {
        return _recordManager;
    }

}
