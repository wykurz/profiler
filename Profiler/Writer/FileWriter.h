#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <fstream>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace Profiler {
namespace Writer {

struct FileWriter {
  FileWriter() : FileWriter(".cxxperf-log", ".") {}
  FileWriter(std::string binaryLogPrefix_, std::string binaryLogDir_)
      : _binaryLogPrefix(std::move(binaryLogPrefix_)),
        _binaryLogDir(std::move(binaryLogDir_)) {}
  template <typename RecordType_>
  void operator()(const RecordType_ & /*record_*/, std::size_t /*holderId_*/,
                  const std::string & /*userContext_*/) {
    DLOG("Saw record type " << typeid(RecordType_).name())
  }

  template <typename RecordType_> std::ofstream &getStream() {
    auto it = _outputs.find(typeid(RecordType_));
    if (it != _outputs.end())
      return it->second;
    auto insPair = _outputs.emplace(
        std::piecewise_construct, std::forward_as_tuple(typeid(RecordType_)),
        std::forward_as_tuple(
            std::ofstream("foo", std::fstream::binary | std::fstream::trunc)));
    const std::string &recordTypeName = typeid(RecordType_).name();
    if (!insPair.second)
      PROFILER_RUNTIME_ERROR("Couldn't add a new output file for type "
                             << recordTypeName);
    auto &out = insPair.first->second;
    DLOG("Setup: " << recordTypeName.size() << " " << recordTypeName << " "
                   << std::size_t(&out))
    const std::size_t &nameSize = recordTypeName.size();
    out.write(reinterpret_cast<const char *>(&nameSize), sizeof(nameSize));
    out << recordTypeName;
    // RecordType_::encodePreamble(out, getManager().id());
  }

private:
  std::string _binaryLogPrefix;
  std::string _binaryLogDir;
  std::unordered_map<std::type_index, std::ofstream> _outputs;
};

// TODO(mateusz): Move those to decoder
// "cxxperf-log.yaml"
// _yamlLogName(std::move(yamlLogName_))

} // namespace Writer
} // namespace Profiler

#endif
