#include <Control/Manager.h>
#include <Instrumentation/StatsScope.h>

namespace Scope
{

    void StatsScope::record()
    {
        auto record = Control::getThread().getRecordManager<Record::Record>().getRecord();
        if (!record.isValid()) return;
        record.get() = Record::Record(_name, _time.delta());
    }

}
