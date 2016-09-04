#include <Instrumentation/StatsScope.h>
#include <Log/Log.h>
#include <Record/Record.h>

namespace Scope
{

    void StatsScope::record()
    {
        auto holder = _recordManager.getRecord();
        if (!holder.isValid()) {
            DLOG() << "No valid holder!";
            return;
        }
        holder.getRecord() = Record::Record(_name, _time.delta());
    }

}
