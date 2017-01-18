#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Profiler/Control/Allocation.h>
#include <Profiler/Control/Arena.h>
#include <Profiler/Control/Holder.h>
#include <Profiler/Config/Config.h>
#include <Profiler/Control/Writer.h>
#include <atomic>
#include <thread>

namespace Profiler { namespace Control
{

    struct Manager
    {
        static constexpr std::size_t MaxThreads = 1024;

        Manager(const Config::Config& config_, bool startWriter_ = true);
        Manager(const Manager&) = delete;
        ~Manager();
        Allocation addThreadRecords();
        void startWriter();
        /**
         * Will stop the writer thread. Can be called multiple times.
         */
        void stopWriter();
        void writerOnePass();
      private:
        Arena _arena{100000};
        Arena _empty{0};
        // TODO: Add alignment and padding?
        std::atomic<int> _currentThread = {0};
        HolderArray _threadArray{MaxThreads};
        std::size_t _droppedThreads = {0};
        FileOutputs _fileOutputs;
        Writer _writer;
        std::thread _writerThread;
        bool _writerStarted = false;
    };

    Manager& getManager();

}
}

#endif
