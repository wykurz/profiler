#include <Profiler/Api.h>

void f1() { Profiler::Scope<Profiler::RdtscClock> scope("f1"); }

void f2() {
  Profiler::Scope<Profiler::SteadyClock> scope("f2");
  f1();
}

void f3() {
  Profiler::Scope<Profiler::SystemClock> scope("f3");
  f2();
  f1();
}

int main() {
  auto config = Profiler::GetDefaultConfig();
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
