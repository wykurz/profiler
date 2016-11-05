#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Control/RecordManager.h>
#include <Control/Writer.h>
#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <thread>

namespace Profiler { namespace Control
{

    struct Thread;

    struct Manager
    {
        static constexpr std::size_t MaxThreads = 1024;

        Manager() = default;
        Manager(const Manager&) = delete;
        ~Manager();

        Arena& addThread(Thread& thread_);

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
        Writer _writer{Output::Ptr(new FileOut("blah")), _threadArray, std::chrono::microseconds(100000)};
        std::thread _writerThread{[this](){ this->_writer.run(); }};
    };

    Manager& getManager();

}
}

#endif
