#include <Profiler/Api.h>

int main() {
  Profiler::DefaultConfig config;
  config.binaryLogPrefix = ".my.perf.log";
  config.binaryLogDir = ".";
  config.yamlLogName = "my.perf.yaml";
  Profiler::setup(config);
  Profiler::primeThisThread();
  auto recordId = PROFILER_EVENT(Rdtsc);
  PROFILER_EVENT(Rdtsc, recordId);
  Profiler::stopProcessor();
  // Profiler::Decoder decoder(config);
  // decoder.run();
  return 0;
}
