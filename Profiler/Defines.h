#ifndef _PROFILER_DEFINES_H
#define _PROFILER_DEFINES_H

#ifdef _MSC_VER
#define PROFILER_FUNCTION __FUNCTION__
#else
#define PROFILER_FUNCTION (const char *)__PRETTY_FUNCTION__
#endif

#endif
