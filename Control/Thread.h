#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <Queue/Queue.h>
#include <Record/Record.h>
#include <Control/RecordManager.h>
#include <cassert>

namespace Control
{

    struct Thread
    {
        Thread();
        Thread(const Thread&) = delete;
        ~Thread();
        template <typename RecordType_>
        RecordManager<RecordType_>& getRecordManager();
      private:
        RecordManager<Record::Record> _recordManager;
    };

    Thread& getThread();

}

#endif
