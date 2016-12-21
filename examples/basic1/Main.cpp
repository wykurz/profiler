#include <Profiler/Decoder/Decoder.h>
#include <Profiler/Instrumentation/StatsScope.h>

void f1()
{
    STATS_SCOPE();
}

int main()
{
    f1();
    return 0;
}
