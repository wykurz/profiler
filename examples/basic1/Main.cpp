#include <Control/Thread.h>
#include <Instrumentation/StatsScope.h>

void f1()
{
    STATS_SCOPE();
}

int main()
{
    f1();
    return 0;
}
