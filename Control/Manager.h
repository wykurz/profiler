#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Control/Store.h>
#include <Queue/Queue.h>

namespace Control
{

    using Queue = Queue::Queue<StorableHolder>;

    struct Thread
    {
        Thread();
        Thread(const Thread&) = delete;
        Queue queue;
    };

    Thread& getThread();

}

#endif
