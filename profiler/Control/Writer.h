#ifndef CONTROL_WRITER_H
#define CONTROL_WRITER_H

#include <Control/Thread.h>
#include <fstream>
#include <memory>
#include <mutex>

namespace Control
{

    struct Output
    {
        using Ptr = std::unique_ptr<Output>;
        virtual std::ostream& get() = 0;
    };

    struct FileOut : Output
    {
        FileOut(const char* name_)
          : _out(name_, std::fstream::binary | std::fstream::trunc)
        { }
        virtual std::ostream& get()
        {
            return _out;
        }
      private:
        std::ofstream _out;
    };

    struct Writer
    {
        Writer(Output::Ptr out_, ThreadArray& threadArray_, std::chrono::microseconds sleepTime_);
        Writer(const Writer&) = delete;
        void run();
        void stop();
      private:
        const Output::Ptr _out;
        ThreadArray& _threadArray;
        const std::chrono::microseconds _sleepTime;
        std::atomic<bool> _done{false};
    };

}

#endif
