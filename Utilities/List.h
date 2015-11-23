#include <atomic>

namespace Utilities {

    struct Node
    {
        Node* getNext() const;
        void setNext(Node& node_);

      private:
        Node* next;
    };

}

