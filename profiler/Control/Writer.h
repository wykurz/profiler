#ifndef CONTROL_WRITER_H
#define CONTROL_WRITER_H

#include <Control/Manager.h>
#include <atomic>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>

namespace Profiler { namespace Control
{

    struct Output
    {
        using Ptr = std::unique_ptr<Output>;
        virtual ~Output() = default;
        virtual std::ostream& get() = 0;
    };

    struct FileOut : Output
    {
        FileOut(const std::string& name_)
          : _out(name_, std::fstream::binary | std::fstream::trunc)
        { }
        virtual std::ostream& get()
        {
            return _out;
        }
      private:
        std::ofstream _out;
    };

    struct Holder
    {
        std::unique_lock<std::mutex> lock()
        {
            return std::unique_lock<std::mutex>(*_lock);
        }
        RecordExtractor* recordExtractor = nullptr;
      private:
        std::unique_ptr<std::mutex> _lock = std::make_unique<std::mutex>();
    };

    struct Allocation
    {
        Allocation(std::unique_lock<std::mutex>&& lock_, Arena& arena_, Holder& holder_)
          : _lock(std::move(lock_)), _arena(arena_), _holder(&holder_)
        { }
        Allocation()
          : _arena(empty()), _holder(nullptr)
        { }
        Arena& getArena() const
        {
            return _arena;
        }
        void setRecordExtractor(RecordExtractor& recordExtractor_) const
        {
            if (_holder) _holder->recordExtractor = &recordExtractor_;
        }
      private:
        static Arena& empty()
        {
            static Arena empty(0);
            return empty;
        }
        std::unique_lock<std::mutex> _lock;
        Arena& _arena;
        Holder* const _holder;
    };

    using HolderArray = std::vector<Holder>;

    /**
     * Writer is responsible for collecting data from all the threads and writing it to the output.
     */
    struct Writer
    {
        /**
         * Writer takes output pointer, global thread array and time interval of how long it should sleep between each
         * activity period.
         */
        Writer(Output::Ptr out_, HolderArray& threadArray_, std::chrono::microseconds sleepTime_);
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
        HolderArray& _threadArray;
        const std::chrono::microseconds _sleepTime;
        std::atomic<bool> _done{false};
    };

}
}

#endif
