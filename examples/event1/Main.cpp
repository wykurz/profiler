#include <Profiler/Api.h>

int main() {
  Profiler::DefaultConfig config;
  config.binaryLogPrefix = ".my.perf.log";
  config.binaryLogDir = ".";
  config.yamlLogName = "my.perf.yaml";
  Profiler::setup(config);
  Profiler::primeThisThread();
  auto recordId = Profiler::eventRecord<Profiler::RdtscClock>("foo");
  Profiler::eventRecord<Profiler::RdtscClock>("bar", recordId);
  Profiler::stopProcessor();
  // Profiler::Decoder decoder(config);
  // decoder.run();
  return 0;
}
