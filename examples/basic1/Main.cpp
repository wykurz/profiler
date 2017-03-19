#include <Profiler/Api.h>

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
  Profiler::Config config;
  config.binaryLogPrefix = ".my.perf.log";
  config.binaryLogDir = ".";
  config.yamlLogName = "my.perf.yaml";
  Profiler::setup(config);
  Profiler::primeThisThread();
  f3();
  Profiler::stopWriter();
  Profiler::Decoder decoder(config);
  decoder.run();
  return 0;
}
