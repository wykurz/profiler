#ifndef QUEUE_QUEUE_H
#define QUEUE_QUEUE_H

#include <algorithm>
#include <atomic>
#include <cassert>
#include <Profiler/Queue/TaggedPtr.h>

namespace Profiler { namespace Queue
{

    template <typename T_>
    struct Queue
    {
        using Type = T_;

        struct Node
        {
            Node() = default;
            Node(const Node& node_) = delete;
            Node(Node&&) = delete;
            Node* getNext() const
            {
                return _next.load(std::memory_order_acquire);
            }
            void setNext(Node* node_)
            {
                return _next.store(node_, std::memory_order_release);
            }
            std::uint32_t getTag() const
            {
                return _tag.load(std::memory_order_acquire);
            }
            Type value;
          private:
            friend Queue<Type>;
            void updateTag()
            {
                // Allow the tag to overflow, we're using unsigned int for that.
                _tag.fetch_add(1, std::memory_order_release);
            }
            std::atomic<Node*> _next{nullptr};
            std::atomic<std::uint32_t> _tag{0};
        };

        Queue(void* basePtr_)
          : _basePtr(static_cast<char*>(basePtr_))
        {
            assert(_head.is_lock_free());
        }

        Node* head() const;
        void push(Node* node_);
        Node* pull();
        Node* extract();
        std::size_t size() const; // Assumes the queue is not modified for the duration of the call.

      private:
        Node* unpackPtr(TaggedPtr<Node> nodePtr_) const
        {
            if (nodePtr_.isNull()) return nullptr;
            auto res = nodePtr_.get(_basePtr);
            return res;
        }

        char* const _basePtr;
        std::atomic<TaggedPtr<Node>> _head{TaggedPtr<Node>()};
    };

    template <typename T_>
    typename Queue<T_>::Node* Queue<T_>::head() const
    {
        return _head.load(std::memory_order_acquire);
    }

    template <typename T_>
    void Queue<T_>::push(Node* node_)
    {
        node_->updateTag();
        auto headPtr = _head.load(std::memory_order_relaxed);
        auto nodePtr = TaggedPtr<Node>(_basePtr, node_);
        do {
            node_->setNext(unpackPtr(headPtr));
        }
        while(!_head.compare_exchange_weak(headPtr, nodePtr,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    }

    template <typename T_>
    typename Queue<T_>::Node* Queue<T_>::pull()
    {
        auto nodePtr = _head.load(std::memory_order_acquire);
        while (!nodePtr.isNull() && !_head.compare_exchange_weak(
                   nodePtr, TaggedPtr<Node>(_basePtr, unpackPtr(nodePtr)->getNext()),
                   std::memory_order_release, std::memory_order_relaxed))
            ; // empty
        if (nodePtr.isNull()) return nullptr;
        auto node = unpackPtr(nodePtr);
        node->setNext(nullptr);
        return node;
    }

    template <typename T_>
    typename Queue<T_>::Node* Queue<T_>::extract()
    {
        auto nodePtr = _head.exchange(TaggedPtr<Node>(), std::memory_order_acq_rel);
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
            node = node->getNext();
        }
        return size;
    }

}
}

#endif
