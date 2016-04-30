#ifndef QUEUE_QUEUE_H
#define QUEUE_QUEUE_H

#include <algorithm>
#include <atomic>

namespace Queue
{

    template<typename T_>
    struct Node
    {
        using Type = T_;
        Node() = default;
        Node(const Type& value_)
          : value(value_)
        { }
        Node(Type&& value_)
          : value(std::move(value_))
        { }
        Node(const Node&) = default;
        Node(Node&&) = default;
        Node* next = nullptr; // Not atomic, assumes no concurrency!
        Type value;
    };

    template<typename T_>
    struct Queue
    {
        using Type = T_;
        using NodeType = Node<T_>;
        void push(NodeType* node_);
        NodeType* pull();
        NodeType* extract();
        std::size_t size() const; // Assumes the list is static for its duration.
      private:
        std::atomic<NodeType*> _head{nullptr};
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
    typename Queue<T_>::NodeType* Queue<T_>::pull()
    {
        // TODO: deal with ABA
        //       - set a base pointer
        //       - store an offset + seq. number
        auto res = _head.load(std::memory_order_acquire);
        while (res && !_head.compare_exchange_weak(res, res->next))
            ; // empty
        if (res) res->next = nullptr;
        return res;
    }

    template<typename T_>
    typename Queue<T_>::NodeType* Queue<T_>::extract()
    {
        return _head.exchange(nullptr, std::memory_order_acquire);
    }

    template<typename T_>
    std::size_t Queue<T_>::size() const
    {
        // Should be used for testing purposes only. It's worth remembering that computing the size of a lock-free list
        // this way may results in a number of elements that this list had never contained at any point in time.
        std::size_t res = 0;
        auto node = _head.load();
        while (node) {
            ++res;
            node = node->next;
        }
        return res;
    }

}

#endif
