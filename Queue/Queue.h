#ifndef QUEUE_QUEUE_H
#define QUEUE_QUEUE_H

#include <algorithm>
#include <atomic>
#include <cassert>

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
        Node* next{nullptr}; // Not atomic, assumes no concurrency!
        Type value;
      private:
        friend struct Queue<Type>;
        void updateTag()
        {
            tag += 1;
            tag &= std::numeric_limits<std::uint32_t>::max() / 2;
        }
        std::uint32_t tag = 0;
    };

    template <typename T_>
    struct Queue
    {
        using Type = T_;
        using NodeType = Node<T_>;
        Queue(const NodeType* baseNode_, std::size_t size_)
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
        struct alignas(sizeof(std::intptr_t)) NodePtr
        {
            NodePtr() = default;
            NodePtr(const NodeType* base_, const NodeType* node_)
              : offset(getOffset(base_, node_)),
                tag(getTag(node_))
            {
                assert(base_);
                assert(!node_ || base_ <= node_);
            }
            NodePtr(const NodePtr&) = default;
            NodeType* applyOffset(const NodeType* baseNode_) const
            {
                return reinterpret_cast<NodeType*>(reinterpret_cast<std::intptr_t>(baseNode_) + offset);
            }
            bool isNull() const
            {
                return -1 == offset;
            }
            static std::uint32_t getOffset(const NodeType* base_, const NodeType* node_)
            {
                if (!node_) return -1;
                return static_cast<std::uint32_t>(reinterpret_cast<std::intptr_t>(node_) - reinterpret_cast<std::intptr_t>(base_));
            }
            static std::uint32_t getTag(const NodeType* node_)
            {
                if (!node_) return 0;
                return node_->tag;
            }
            const std::uint32_t offset = -1;
            const std::uint32_t tag = 0;
        };
        static_assert(sizeof(std::intptr_t) == sizeof(NodePtr), "Tagged pointer doesn't match the expected size!");
        NodeType* unpackPtr(NodePtr nodePtr_) const
        {
            if (nodePtr_.isNull()) return nullptr;
            auto res = nodePtr_.applyOffset(_baseNode);
            assert(res < _baseNode + _size);
            return res;
        }
        const NodeType* const _baseNode;
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
