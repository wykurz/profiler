#include <Config/Config.h>
#include <Control/Manager.h>
#include <Control/Thread.h>
#include <Log/Log.h>
#include <array>
#include <atomic>

namespace Profiler { namespace Control
{

    Manager::Manager(const Config::Config& config_)
      : _writer(Output::Ptr(new FileOut(config_.logFileName)), _threadArray, std::chrono::microseconds(100000))
    { }

    Manager::~Manager()
    {
        stopWriter();
        _writerThread.join();
    }

    ThreadAllocation Manager::addThread()
    {
        int count = MaxThreads;
        while (0 < count--) {
            auto& holder = _threadArray[_currentThread++];
            auto lk = holder.lock();
            if (holder.thread) continue;
            return {std::move(lk), _arena, holder};
        }
        ++_droppedThreads;
        return {};
    }

    void Manager::stopWriter()
    {
        _writer.stop();
    }

    Manager& getManager()
    {
        static Manager manager(Config::getConfig());
        return manager;
    }

}
}
