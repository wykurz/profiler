#ifndef MANAGER_H
#define MANAGER_H

namespace Control
{

    struct Thread
    {
        Thread();
        Thread(const Thread&) = delete;
        Queue queue;
    };

    Thread& getThread();

}

#endif
