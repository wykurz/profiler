#include <Profiler/Decoder/Decoder.h>
#include <Profiler/Instrumentation/StatsScope.h>

void f1() { STATS_SCOPE(); }

void f2() {
  STATS_SCOPE();
  f1();
}

void f3() {
  STATS_SCOPE();
  f2();
  f1();
}

int main() {
  auto logFilePrefix = ".my.perf.log";
  auto binaryLogDir = ".";
  auto yamlFileName = "my.perf.yaml";
  Profiler::Config::setConfig(Profiler::Config::Config(logFilePrefix, binaryLogDir, yamlFileName));
  Profiler::Control::primeThreadRecords();
  f3();
  Profiler::Control::getManager().stopWriter();
  Profiler::Decoder::Decoder decoder(Profiler::Config::getConfig());
  ;
  decoder.run();
  return 0;
}
