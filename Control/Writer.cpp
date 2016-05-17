#include <Control/Writer.h>
#include <mutex>

namespace Control
{

    Writer::Writer(Output::Ptr out_)
      : _out(std::move(out_))
    { }

    void Writer::run(ThreadArray& threadArray_)
    {
        for (auto& holder : threadArray_) {
            auto lk = holder.lock();
            if (!holder.thread) continue;
            auto& thread = *(holder.thread);
            auto recordNode = thread.template getRecordManager<Record::Record>().extractDirtyRecords();
            while (!recordNode) {
                _out->get() << recordNode->value;
                recordNode = recordNode->next;
            }
        }
    }

}
