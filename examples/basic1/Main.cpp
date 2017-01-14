#include <Profiler/Decoder/Decoder.h>
#include <Profiler/Instrumentation/StatsScope.h>

void f1()
{
    STATS_SCOPE();
}

int main()
{
    using namespace Profiler;
    const char* logFilePrefix = ".my.perf.log";
    Config::setConfig(Config::Config(logFilePrefix));
    f1();
    Control::getManager().stopWriter();
    Decoder::Decoder decoder(Config::getConfig());;
    decoder.run();
    return 0;
}
