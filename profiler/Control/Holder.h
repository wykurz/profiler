#ifndef CONTROL_HOLDER_H
#define CONTROL_HOLDER_H

#include <Control/RecordManager.h>
#include <memory>
#include <mutex>

namespace Profiler { namespace Control
{

    struct Holder
    {
        std::unique_lock<std::mutex> lock()
        {
            return std::unique_lock<std::mutex>(*_lock);
        }
        RecordExtractor* recordExtractor = nullptr;
      private:
        std::unique_ptr<std::mutex> _lock = std::make_unique<std::mutex>();
    };

}
}

#endif
