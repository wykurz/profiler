#include <Profiler/Config/Config.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/Writer.h>
#include <Profiler/Exception/Exception.h>
#include <chrono>

namespace Profiler { namespace Control
{

    // TODO: The Writer needs to be opening the log files using a name pattern
    Manager::Manager(const Config::Config& config_, bool startWriter_)
      : _fileOutputs(config_), _writer(_threadArray, std::chrono::microseconds(100000))
    {
        if (startWriter_) {
            _writerThread = std::thread([this](){ this->_writer.run(); });
            _writerStarted = true;
        }
    }

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
            // TODO: Setup output streams here...
            return {std::move(lk), _arena, holder, _fileOutputs};
        }
        ++_droppedThreads;
        return {};
    }

    void Manager::startWriter()
    {
        if (_writerStarted) return;
        _writerThread = std::thread([this](){ this->_writer.run(); });
        _writerStarted = true;
    }

    void Manager::stopWriter()
    {
        if (!_writerStarted) return;
        _writer.stop();
        if (_writerThread.joinable()) _writerThread.join();
        _writerStarted = false;
    }

    void Manager::writerOnePass()
    {
        PROFILER_ASSERT(!_writerStarted);
        _writer.onePass();
    }

    Manager& getManager()
    {
        static Manager manager(Config::getConfig());
        return manager;
    }

}
}
