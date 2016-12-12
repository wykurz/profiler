#include <Control/Writer.h>
#include <Exception/Exception.h>
#include <chrono>
#include <thread>
#include <utility>

namespace Profiler { namespace Control
{

    Writer::Writer(Output::Ptr out_, HolderArray& threadArray_, std::chrono::microseconds sleepTime_)
      : _out(std::move(out_)),
        _threadArray(threadArray_),
        _sleepTime(sleepTime_)
    {
        PROFILER_ASSERT(_out.get());
    }

    void Writer::run()
    {
        do {
            for (auto& holder : _threadArray) {
                auto lk = holder.lock();
                if (!holder.recordExtractor) continue;
                auto& recordExtractor = *(holder.recordExtractor);
                recordExtractor.streamDirtyRecords(_out->get());
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
