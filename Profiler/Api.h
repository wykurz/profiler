#ifndef _PROFILER_API_H
#define _PROFILER_API_H

#include <Profiler/Config.h>
#include <Profiler/Decoder.h>
#include <Profiler/Instrumentation/StatsScope.h>

namespace Profiler {

inline void setup(const Config &config_) { Config::setConfig(config_); }
inline void primeThreadRecords() { Profiler::Control::primeThreadRecords(); }
inline void stopWriter() { Profiler::Control::getManager().stopWriter(); }
} // namespace Profiler

#endif
