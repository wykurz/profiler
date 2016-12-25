#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <Profiler/Record/Record.h>
#include <Profiler/Control/Manager.h>
#include <Profiler/Control/RecordManager.h>
#include <cassert>
#include <memory>
#include <mutex>
#include <vector>

namespace Profiler { namespace Control
{

    template <typename Record_>
    struct ThreadRecords
    {
        using RecordManagerType = RecordManager<Record_>;
        ThreadRecords(const Allocation& allocation_)
          : _recordManager(allocation_.getArena()),
            _finalizer(allocation_.setRecordExtractor(_recordManager))
        { }
        ThreadRecords(const ThreadRecords&) = delete;
        ~ThreadRecords()
        {
            // TODO: Grab a slot-lock and deregister ourselves from Manager's thread buffer list
        }
        RecordManagerType& getRecordManager()
        {
            return _recordManager;
        }

      private:
        RecordManager<Record_> _recordManager;
        Finalizer _finalizer;
    };

    template <typename Record_>
    ThreadRecords<Record_>& getThreadRecords()
    {
        thread_local ThreadRecords<Record_> threadRecords(getManager().addThreadRecords());
        return threadRecords;
    }

}
}

#endif