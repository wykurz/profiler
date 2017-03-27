#ifndef _PROFILER_API_H
#define _PROFILER_API_H

#include <Profiler/Config.h>
#include <Profiler/Decoder.h>
#include <Profiler/Instrumentation/ProfilerScope.h>

namespace Profiler {

inline void setup(const Config &config_) { Config::setConfig(config_); }
inline void primeThisThread() { Profiler::Control::primeThisThread(); }
inline void stopWriter() { Profiler::Control::getManager().stopWriter(); }
} // namespace Profiler

#endif
