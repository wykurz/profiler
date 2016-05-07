#ifndef CONTROL_WRITER_H
#define CONTROL_WRITER_H

#include <Control/Thread.h>
#include <mutex>

namespace Control
{

    struct Writer
    {
        Writer() = default;
        Writer(const Writer&) = delete;
        void run(ThreadArray& threadArray_)
        {
            for (auto& threadHolder : threadArray_) {
                std::unique_lock<std::mutex> lk(threadHolder.lock);
                if (!threadHolder.thread) continue;
                auto& thread = *threadHolder.thread;
                auto recordNode = thread.template getRecordManager<Record::Record>().extractDirtyRecords();
                if (!recordNode) continue;
                // TODO
            }
        }
    };

}

#endif
