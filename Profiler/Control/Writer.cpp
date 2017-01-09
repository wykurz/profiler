#include <Profiler/Control/Writer.h>
#include <Profiler/Exception/Exception.h>
#include <chrono>
#include <thread>
#include <utility>

namespace Profiler { namespace Control
{

    Writer::Writer(HolderArray& threadArray_, std::chrono::microseconds sleepTime_)
      : _threadArray(threadArray_),
        _sleepTime(sleepTime_)
    { }

    Writer::~Writer()
    {
        PROFILER_ASSERT(_done.load(std::memory_order_acquire));
        // One final run to capture any events that may have been missed due to notification timing
        run();
    }

    void Writer::run()
    {
        do {
            for (auto& holder : _threadArray) {
                auto lk = holder.lock();
                holder.getPtr()->streamDirtyRecords();
            }
            std::this_thread::sleep_for(_sleepTime);
        }
        while (!_done.load(std::memory_order_acquire));
    }

    void Writer::stop()
    {
        _done.store(true, std::memory_order_release);
    }

}
}
