#ifndef CONTROL_WRITER_H
#define CONTROL_WRITER_H

#include <Profiler/Control/Allocation.h>
#include <Profiler/Control/Holder.h>
#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <string>

namespace Profiler { namespace Control
{

    /**
     * Writer is responsible for collecting data from all the threads and writing it to the output.
     */
    struct Writer
    {
        /**
         * Writer takes output pointer, global thread array and time interval of how long it should sleep between each
         * activity period.
         */
        Writer(HolderArray& threadArray_, std::chrono::microseconds sleepTime_);
        Writer(const Writer&) = delete;
        ~Writer();
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
        HolderArray& _threadArray;
        const std::chrono::microseconds _sleepTime;
        std::atomic<bool> _done{false};
    };

}
}

#endif
