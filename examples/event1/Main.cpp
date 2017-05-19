#include <Profiler/Api.h>

int main() {
  Profiler::Config config;
  config.binaryLogPrefix = ".my.perf.log";
  config.binaryLogDir = ".";
  config.yamlLogName = "my.perf.yaml";
  Profiler::setup(config);
  Profiler::primeThisThread();
  auto recordId = Profiler::eventRecord<Profiler::Clock::Rdtsc>("Foo");
  Profiler::eventRecord<Profiler::Clock::Rdtsc>("Bar", recordId);
  Profiler::stopWriter();
  Profiler::Decoder decoder(config);
  decoder.run();
  return 0;
}