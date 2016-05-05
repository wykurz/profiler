#include <Control/Manager.h>
#include <array>
#include <atomic>

namespace Control
{

    Manager& getManager()
    {
        static Manager manager;
        return manager;
    }

}
