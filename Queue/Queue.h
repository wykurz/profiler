#ifndef QUEUE_QUEUE_H
#define QUEUE_QUEUE_H

#include <atomic>

namespace Queue
{

    template<typename T_>
    struct Node
    {
        using Type = T_;
        Node(Type&& value_)
          : value(std::move(value_))
        { }

        Node* next = nullptr;
        Type value;
    };

    template<typename T_>
    struct Queue
    {
        using Type = T_;
        using NodeType = Node<T_>;
        void push(NodeType* node_);
        NodeType* extract();

      private:
        std::atomic<NodeType*> _head;
    };

    template<typename T_>
    void Queue<T_>::push(NodeType* node_)
    {
        // Inspired by: http://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
        node_->next = _head.load(std::memory_order_relaxed);
        while(!_head.compare_exchange_weak(node_->next, node_,
                                           std::memory_order_release,
                                           std::memory_order_relaxed))
            ; // empty
    }

    template<typename T_>
    typename Queue<T_>::NodeType* Queue<T_>::extract()
    {
        return _head.exchange(nullptr, std::memory_order_acquire);
    }

}

#endif
