#ifndef EXCEPTION_EXCEPTION_H
#define EXCEPTION_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace Profiler { namespace Exception
{

    struct Runtime: public std::runtime_error
    {
        Runtime(std::string message_)
          : std::runtime_error(message_)
        { }
    };

    struct LogicError: public std::logic_error
    {
        LogicError(std::string message_)
          : std::logic_error(message_)
        { }
    };

}
}

// TODO: Replace all assertions with this?

#ifdef DEBUG
#define PROFILER_ASSERT(x) if (!(x)) throw LogicError(__PRETTY_FUNCTION__);
#else
#define PROFILER_ASSERT(x) do { } while (false);
#endif

#endif
