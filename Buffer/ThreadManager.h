#include <Buffer/Buffer.h>
#include <memory>

namespace Buffer {

    struct ThreadManager
    {

      private:
        std::unique_ptr<Buffer> _current;
    };

}
