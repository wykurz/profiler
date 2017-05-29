#include <Profiler/Api.h>

void f1() { PROFILER_SCOPE(Rdtsc); }

void f2() {
  PROFILER_SCOPE(Steady);
  f1();
}

void f3() {
  PROFILER_SCOPE(Steady);
  f2();
  f1();
}

int main() {
  Profiler::DefaultConfig config;
  config.binaryLogPrefix = ".my.perf.log";
  config.binaryLogDir = ".";
  config.yamlLogName = "my.perf.yaml";
  Profiler::setup(config);
  Profiler::primeThisThread();
  f3();
  Profiler::stopProcessor();
  // auto decoder = Profiler::Decoder(config);
  // decoder.run();
  return 0;
}
