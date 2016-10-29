#ifndef QUEUE_QUEUE_H
#define QUEUE_QUEUE_H

#include <algorithm>
#include <atomic>
#include <cassert>
#include <Queue/TaggedPtr.h>

namespace Queue
{

    template <typename T_>
    struct Queue;

    template <typename T_>
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
        Node& operator=(const Node&) = default;
        Node(Node&&) = default;
        std::uint32_t getTag() const
        {
            return tag;
        }
        Node* next{nullptr}; // Assumes no concurrency!
        Type value;
      private:
        friend struct Queue<Type>;
        void updateTag()
        {
            // Allow the tag to overflow, we're using unsigned int for that:
            tag += 1;
        }
        std::uint32_t tag = 0;
    };

    template <typename T_>
    struct Queue
    {
        using Type = T_;
        using NodeType = Node<T_>;
        Queue(NodeType* baseNode_, std::size_t size_)
          : _baseNode(baseNode_),
            _size(size_)
        {
            assert(_head.is_lock_free());
        }
        void push(NodeType* node_);
        NodeType* pull();
        NodeType* extract();
        std::size_t size() const; // Assumes the queue is static for the duration of the call.
      private:
        // TODO: run clang undef. behavior sanitizer
        using NodePtr = TaggedPtr<Node<T_>>;
        NodeType* unpackPtr(NodePtr nodePtr_) const
        {
            if (nodePtr_.isNull()) return nullptr;
            auto res = nodePtr_.get(_baseNode);
            assert(res < _baseNode + _size);
            return res;
        }
        NodeType* const _baseNode;
        const std::size_t _size;
        std::atomic<NodePtr> _head{NodePtr()};
    };

    template <typename T_>
    void Queue<T_>::push(NodeType* node_)
    {
        node_->updateTag();
        auto headPtr = _head.load(std::memory_order_relaxed);
        auto nodePtr = NodePtr(_baseNode, node_);
        do {
            node_->next = unpackPtr(headPtr);
        }
        while(!_head.compare_exchange_weak(headPtr, nodePtr,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    }

    template <typename T_>
    typename Queue<T_>::NodeType* Queue<T_>::pull()
    {
        auto nodePtr = _head.load(std::memory_order_acquire);
        while (!nodePtr.isNull() && !_head.compare_exchange_weak(nodePtr, NodePtr(_baseNode, unpackPtr(nodePtr)->next)))
            ; // empty
        if (nodePtr.isNull()) return nullptr;
        auto node = unpackPtr(nodePtr);
        node->next = nullptr;
        return node;
    }

    template <typename T_>
    typename Queue<T_>::NodeType* Queue<T_>::extract()
    {
        auto nodePtr = _head.exchange(NodePtr(), std::memory_order_acquire);
        if (nodePtr.isNull()) return nullptr;
        auto node = unpackPtr(nodePtr);
        return node;
    }

    template <typename T_>
    std::size_t Queue<T_>::size() const
    {
        // Should be used for testing purposes only. It's worth remembering that computing the size of a lock-free list
        // this way may results in a number of elements that this list had never contained at any point in time.
        std::size_t size = 0;
        auto nodePtr = _head.load();
        if (nodePtr.isNull()) return 0;
        auto node = unpackPtr(nodePtr);
        while (node) {
            ++size;
            node = node->next;
        }
        return size;
    }

}

#endif
