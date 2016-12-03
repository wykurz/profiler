#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Config/Config.h>
#include <Control/RecordManager.h>
#include <Control/Thread.h>
#include <Control/Writer.h>
#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>
#include <chrono>
#include <thread>

namespace Profiler { namespace Control
{

    struct Manager
    {
        static constexpr std::size_t MaxThreads = 1024;

        Manager(const Config::Config& config_);
        Manager(const Manager&) = delete;
        ~Manager();

        ThreadAllocation addThread();

        /**
         * Use for test purposes - will top the writer thread. The writer thread may not be restarted.
         */
        void stopWriter();

      private:
        Arena _arena{100000};
        Arena _empty{0};
        // TODO: Add alignment and padding?
        std::atomic<int> _currentThread = {0};
        ThreadArray _threadArray{MaxThreads};
        std::size_t _droppedThreads = {0};
        Writer _writer;
        std::thread _writerThread{[this](){ this->_writer.run(); }};
    };

    Manager& getManager();

}
}

#endif
