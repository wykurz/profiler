#include <Control/Manager.h>
#include <Instrumentation/StatsScope.h>

namespace Scope
{

    void StatsScope::record()
    {
        auto holder = Control::getThread().getRecordManager<Record::Record>().getRecord();
        if (!holder.isValid()) return;
        holder.getRecord() = Record::Record(_name, _time.delta());
    }

}
