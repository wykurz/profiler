#ifndef CONTROL_WRITER_H
#define CONTROL_WRITER_H

#include <Control/Thread.h>
#include <fstream>
#include <mutex>

namespace Control
{

    struct Writer
    {
        Writer(const char* file_);
        Writer(const Writer&) = delete;
        void run(ThreadArray& threadArray_);
      private:
        std::ofstream _out;
    };

}

#endif
