#include <Algorithms/Mpl.h>
#include <Control/Manager.h>
#include <Control/ThreadHandling.h>
#include <Control/Writer.h>
#include <Log/Log.h>
#include <cassert>
#include <chrono>
#include <mutex>
#include <thread>
#include <utility>

namespace Profiler { namespace Control
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
                // auto& thread = *(holder.thread);
                // auto extract = [&](auto typeInfo_) {
                //     auto recordNode = thread.template getThreadRecords<typename decltype(typeInfo_)::Type>().extractDirtyRecords();
                //     while (recordNode) {
                //         this->_out->get() << recordNode->value;
                //         recordNode = recordNode->getNext();
                //     }
                // };
                // Mpl::apply<Mpl::TypeList<Record::Record> >(extract);
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
