#ifndef QUEUE_QUEUE_H
#define QUEUE_QUEUE_H

#include <atomic>

namespace Queue
{

    struct Node
    {
        Node* next;
    };

    struct Queue
    {
        void push(Node* node_);
        Node* extract();

      private:
        std::atomic<Node*> _head;
    };

}

#endif
