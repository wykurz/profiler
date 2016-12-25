#include <Profiler/Decoder/Decoder.h>
#include <Profiler/Instrumentation/StatsScope.h>

void f1()
{
    STATS_SCOPE();
}

int main()
{
    using namespace Profiler;
    const char* logFileName = ".perf.log";
    Config::setConfig(Config::Config(logFileName));
    f1();
    Control::getManager().stopWriter();
    Decoder::Decoder decoder(Config::getConfig());;
    decoder.run();
    return 0;
}
