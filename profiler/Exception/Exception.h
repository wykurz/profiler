#ifndef EXCEPTION_EXCEPTION_H
#define EXCEPTION_EXCEPTION_H

#include <stdexcept>

namespace Profiler { namespace Exception
{

    struct Runtime: public std::runtime_error
    {
        Runtime(std::string message_)
          : std::runtime_error(message_)
        { }
    };

}
}

#endif
