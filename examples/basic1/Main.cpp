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
    STATS_SCOPE_EX("A");
    {
        STATS_SCOPE_EX("B");
        {
            STATS_SCOPE_EX("C");
        }
    }

    STATS_SCOPE_EX("D");
    {
        STATS_SCOPE_EX("E");
        {
            STATS_SCOPE_EX("F");
        }
    }

    STATS_SCOPE_EX("G");
    {
        STATS_SCOPE_EX("H");
        {
            STATS_SCOPE_EX("I");
        }
    }

    // f2();
    // f1();
}

int main()
{
    using namespace Profiler;
    auto logFilePrefix = ".my.perf.log";
    auto binaryLogDir = ".";
    auto yamlFileName = "my.perf.yaml";
    Config::setConfig(Config::Config(logFilePrefix, binaryLogDir, yamlFileName));
    Control::primeThreadRecords();
    f3();
    Control::getManager().stopWriter();
    Decoder::Decoder decoder(Config::getConfig());;
    decoder.run();
    return 0;
}
