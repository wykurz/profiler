#include <Queue/Queue.h>
#include <atomic>

namespace Queue
{

    void Queue::push(Node* node_)
    {
        // Inspired by: http://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
        node_->next = _head.load(std::memory_order_relaxed);
        while(!_head.compare_exchange_weak(node_->next, node_,
                                           std::memory_order_release,
                                           std::memory_order_relaxed))
            ; // empty
    }

    Node* Queue::extract()
    {
        return _head.exchange(nullptr, std::memory_order_acquire);
    }

}
