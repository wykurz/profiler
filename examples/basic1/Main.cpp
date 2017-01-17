#include <Profiler/Decoder/Decoder.h>
#include <Profiler/Instrumentation/StatsScope.h>

void f1()
{
    STATS_SCOPE();
}

void f2()
{
    STATS_SCOPE();
    f1();
}

void f3()
{
    STATS_SCOPE();
    f2();
    f1();
}

int main()
{
    using namespace Profiler;
    auto logFilePrefix = ".my.perf.log";
    auto binaryLogDir = ".";
    auto yamlFileName = "my.perf.yaml";
    Config::setConfig(Config::Config(logFilePrefix, binaryLogDir, yamlFileName));
    f3();
    Control::getManager().stopWriter();
    Decoder::Decoder decoder(Config::getConfig());;
    decoder.run();
    return 0;
}
