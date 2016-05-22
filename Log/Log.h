#ifndef LOG_LOG_H
#define LOG_LOG_H

#include <iostream>

// TODO: for release this should compile out
#define DLOG() std::cerr << "\n" << __PRETTY_FUNCTION__ << " :: "

#endif
