#include <Profiler/Config/Config.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/Writer.h>
#include <chrono>

namespace Profiler { namespace Control
{

namespace
{

    struct FileOut : Output
    {
        FileOut(const std::string& name_)
          : _out(name_, std::fstream::binary | std::fstream::trunc)
        { }
        virtual std::ostream& get()
        {
            return _out;
        }
      private:
        std::ofstream _out;
    };

}

    // TODO: The Writer needs to be opening the log files using a name pattern
    Manager::Manager(const Config::Config& config_)
      : _writer(Output::Ptr(new FileOut(config_.binaryLogPrefix)), _threadArray, std::chrono::microseconds(100000))
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
            return {std::move(lk), _arena, holder};
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
