#ifndef MANAGER_H
#define MANAGER_H

#include <Queue/Queue.h>

namespace Control
{

    struct Thread
    {
        Thread();
        Thread(const Thread&) = delete;
        Queue::Queue queue;
    };

    Thread& getThread();

}

#endif
