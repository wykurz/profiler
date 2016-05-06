#include <Control/Manager.h>
#include <Control/Thread.h>
#include <Instrumentation/StatsScope.h>
#include <Record/Record.h>

namespace Scope
{

    void StatsScope::record()
    {
        auto holder = Control::getThread().template getRecordManager<Record::Record>().getRecord();
        if (!holder.isValid()) return;
        holder.getRecord() = Record::Record(_name, _time.delta());
    }

}
