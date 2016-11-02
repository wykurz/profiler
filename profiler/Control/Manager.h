#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Control/RecordManager.h>
#include <Control/Writer.h>
#include <Queue/Queue.h>
#include <Record/Record.h>
#include <cassert>
#include <condition_variable>
#include <thread>

namespace Control
{

    struct Thread;

    struct Manager
    {
        static constexpr std::size_t MaxThreads = 1024;
        static constexpr std::size_t MaxSlotSearches = MaxThreads;

        Manager() = default;
        Manager(const Manager&) = delete;
        ~Manager();

        ThreadHolder* addThread(Thread& thread_);

      private:
        // TODO: Add alignment and padding?
        std::atomic<int> _currentThread = {0};
        ThreadArray _threadArray{MaxThreads};
        std::size_t _droppedThreads = {0};
        Writer _writer{Output::Ptr(new FileOut("blah")), _threadArray};
        std::thread _writerThread{[this](){ this->_writer.run(); }};
    };

    Manager& getManager();

}

#endif
