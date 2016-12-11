#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <Record/Record.h>
#include <Control/Manager.h>
#include <Control/RecordManager.h>
#include <cassert>
#include <memory>
#include <mutex>
#include <vector>

namespace Profiler { namespace Control
{

    template <typename Record_>
    struct ThreadRecords : ThreadRecordExtractor
    {
        using RecordManagerType = RecordManager<Record_>;
        ThreadRecords(const ThreadAllocation& allocation_)
          : _recordManager(allocation_.getArena())
        {
            allocation_.setThread(*this);
        }
        ThreadRecords(const ThreadRecords&) = delete;
        virtual ~ThreadRecords()
        {
            // TODO: Grab a slot-lock and deregister ourselves from Manager's thread buffer list
        }
        virtual RecordExtractor& getRecordExtractor() override
        {
            return _recordManager;
        }
        RecordManagerType& getRecordManager()
        {
            return _recordManager;
        }

      private:
        RecordManager<Record_> _recordManager;
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
