#include <Profiler/Api.h>

int main() {
  auto config = Profiler::GetDefaultConfig();
  Profiler::setup(config, Profiler::FileWriter<decltype(config)::RecordList>());
  Profiler::primeThisThread();
  auto recordId = Profiler::eventRecord<Profiler::RdtscClock>("foo");
  Profiler::eventRecord<Profiler::RdtscClock>("bar", recordId);
  Profiler::stopProcessor();
  return 0;
}
