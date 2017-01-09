#include <Profiler/Config/Config.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/Writer.h>
#include <chrono>

namespace Profiler { namespace Control
{

    // TODO: The Writer needs to be opening the log files using a name pattern
    Manager::Manager(const Config::Config& config_)
      : _fileOutputs(config_), _writer(_threadArray, std::chrono::microseconds(100000))
    { }

    Manager::~Manager()
    {
        stopWriter();
    }

    Allocation Manager::addThreadRecords()
    {
        int count = MaxThreads;
        while (0 < count--) {
            auto& holder = _threadArray[_currentThread++];
            auto lk = holder.lock();
            if (!holder.isEmpty()) continue;
            return {std::move(lk), _arena, holder, _fileOutputs};
        }
        ++_droppedThreads;
        return {};
    }

    void Manager::stopWriter()
    {
        _writer.stop();
        if (_writerThread.joinable()) _writerThread.join();
    }

    Manager& getManager()
    {
        static Manager manager(Config::getConfig());
        return manager;
    }

}
}
