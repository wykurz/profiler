#include <Control/Writer.h>
#include <Log/Log.h>
#include <cassert>
#include <chrono>
#include <mutex>
#include <thread>

namespace Control
{

    Writer::Writer(Output::Ptr out_, ThreadArray& threadArray_, std::chrono::microseconds sleepTime_)
      : _out(std::move(out_)),
        _threadArray(threadArray_),
        _sleepTime(sleepTime_)
    {
        assert(_out.get());
    }

    void Writer::run()
    {
        do {
            for (auto& holder : _threadArray) {
                auto lk = holder.lock();
                if (!holder.thread) continue;
                auto& thread = *(holder.thread);
                auto recordNode = thread.template getRecordManager<Record::Record>().extractDirtyRecords();
                while (recordNode) {
                    _out->get() << recordNode->value;
                    recordNode = recordNode->next;
                }
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
