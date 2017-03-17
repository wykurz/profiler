#ifndef _PROFILER_EXCEPTION_EXCEPTION_H
#define _PROFILER_EXCEPTION_EXCEPTION_H

#include <Profiler/Defines.h>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Profiler {
namespace Exception {

struct Runtime : public std::runtime_error {
  explicit Runtime(std::string message_)
      : std::runtime_error(std::move(message_)) {}
};

struct LogicError : public std::logic_error {
  explicit LogicError(std::string message_)
      : std::logic_error(std::move(message_)) {}
};
} // namespace Exception
} // namespace Profiler

#define PROFILER_RUNTIME_ERROR(message_)                                       \
  do {                                                                         \
    std::stringstream ss;                                                      \
    ss << "\n\nRuntimeError in:\n"                                             \
       << PROFILER_FUNCTION << "\n"                                            \
       << "" << message_ << "\n";                                              \
    throw Profiler::Exception::Runtime(ss.str());                              \
  } while (false);

#ifdef DEBUG
#define PROFILER_ASSERT(x)                                                     \
  if (!(x)) {                                                                  \
    std::stringstream ss;                                                      \
    ss << "\n\nLogic error in:\n"                                              \
       << __FILE__ << ":" << __LINE__ << "\n  " << PROFILER_FUNCTION           \
       << "\nThe following is false: ( " << #x << " )";                        \
    throw Profiler::Exception::LogicError(ss.str());                           \
  }
#else
#define PROFILER_ASSERT(x)                                                     \
  if (x) {                                                                     \
  }
#endif

#endif
