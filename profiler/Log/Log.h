#ifndef LOG_LOG_H
#define LOG_LOG_H

#include <iostream>

#ifdef DEBUG
#define DLOG(x) do { std::cerr << __PRETTY_FUNCTION__ << " :: " << x << std::endl; } while (false);
#else
#define DLOG(x) do { } while (false);
#endif

#endif
