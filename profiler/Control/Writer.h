#ifndef CONTROL_WRITER_H
#define CONTROL_WRITER_H

#include <Control/Thread.h>
#include <fstream>
#include <memory>
#include <mutex>

namespace Profiler { namespace Control
{

    struct Output
    {
        virtual ~Output() = default;
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

    /**
     * Writer is responsible for collecting data from all the threads and writing it to the output.
     */
    struct Writer
    {
        /**
         * Writer takes output pointer, global thread array and time interval of how long it should sleep between each
         * activity period.
         */
        Writer(Output::Ptr out_, ThreadArray& threadArray_, std::chrono::microseconds sleepTime_);
        Writer(const Writer&) = delete;
        /**
         * Puts worker in a loop periodically checking if any thread produced output that needs to be written to disk.
         * After iterating through all threads, worker will sleep for a fixed amount of time.
         */
        void run();
        /**
         * The run loop will eventually terminate after stop() was called.
         */
        void stop();
      private:
        const Output::Ptr _out;
        ThreadArray& _threadArray;
        const std::chrono::microseconds _sleepTime;
        std::atomic<bool> _done{false};
    };

}
}

#endif
