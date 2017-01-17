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

#define PROFILER_RUNTIME_ERROR(message_) do {                         \
        std::stringstream ss;                                         \
        ss << "\n\nRuntimeError in:\n" << __PRETTY_FUNCTION__ << "\n" \
           << "" << message_ << "\n";                                 \
        throw Profiler::Exception::Runtime(ss.str());                 \
    } while(false);


// TODO: Replace all assertions with this.

#ifdef DEBUG
#define PROFILER_ASSERT(x) if (!(x)) {                       \
        std::stringstream ss;                                \
        ss << "\n\nLogic error in:\n" << __PRETTY_FUNCTION__ \
           << "\nThe following is false: ( " << #x << " )";  \
        throw Profiler::Exception::LogicError(ss.str());     \
    }
#else
#define PROFILER_ASSERT(x) do { } while (false);
#endif

#endif
