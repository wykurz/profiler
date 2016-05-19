#include <Control/Writer.h>
#include <Log/Log.h>
#include <cassert>
#include <mutex>

namespace Control
{

    Writer::Writer(Output::Ptr out_)
      : _out(std::move(out_))
    {
        assert(_out.get());
    }

    void Writer::run(ThreadArray& threadArray_)
    {
        DLOG() << "TEST";
        for (auto& holder : threadArray_) {
            DLOG() << "here1";
            auto lk = holder.lock();
            if (!holder.thread) continue;
            DLOG() << "here2";
            auto& thread = *(holder.thread);
            auto recordNode = thread.template getRecordManager<Record::Record>().extractDirtyRecords();
            while (recordNode) {
                DLOG() << "here3";
                _out->get() << recordNode->value;
                recordNode = recordNode->next;
            }
        }
    }

}
