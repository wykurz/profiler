#ifndef EXCEPTION_EXCEPTION_H
#define EXCEPTION_EXCEPTION_H

#include <stdexcept>
#include <sstream>
#include <string>

namespace Profiler { namespace Exception
{

    struct Runtime: public std::runtime_error
    {
        Runtime(std::string message_)
          : std::runtime_error(std::move(message_))
        { }
    };

    struct LogicError: public std::logic_error
    {
        LogicError(std::string message_)
          : std::logic_error(std::move(message_))
        { }
    };

}
}

// TODO: Replace all assertions with this?

#ifdef DEBUG
#define PROFILER_ASSERT(x) if (!(x)) {                   \
        std::stringstream ss;                            \
        ss << __PRETTY_FUNCTION__ << " : " << #x;        \
        throw Profiler::Exception::LogicError(ss.str()); \
    }
#else
#define PROFILER_ASSERT(x) do { } while (false);
#endif

#endif
