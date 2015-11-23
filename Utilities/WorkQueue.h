#include <atomic>

namespace Utilities {

    template <typename NodeType_>
    struct WorkQueue
    {
        using NodeType = NodeType_;
        void push(NodeType elem_);
        T grab();
        NodeType* extract();

      private:
        std::atomic<Node*> _head;
    };

}
