#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <Profiler/Record/Records.h>
#include <Profiler/Serialize.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Profiler {
namespace Writer {

namespace fs = boost::filesystem;

namespace Internal {

template <typename StorageType_>
inline void decodeStream(std::istream &in_, std::ostream &out_) {
  StorageType_::decodePreamble(in_, out_);
  out_ << "records:\n";
  while (in_.good() && in_.peek() != EOF) {
    DLOG("Currently at: " << in_.tellg());
    StorageType_::decode(in_, out_);
  }
}
} // namespace Internal

template <typename RecordList_> struct Decoder {
  using RecordList = RecordList_;
  using DecodeFunc = std::function<void(std::istream &, std::ostream &)>;
  explicit Decoder(const std::string &binaryLogDir_,
                   const std::string &binaryLogPrefix_,
                   const std::string &yamlLogName_)
      : _funcMap(genFuncMap<RecordList>()),
        _out(yamlLogName_, std::fstream::trunc) {
    const fs::path logDir(binaryLogDir_);
    DLOG("Traversing: " << logDir);
    for (auto &entry : fs::directory_iterator(logDir)) {
      if (!fs::is_regular_file(entry.status())) {
        DLOG("Not a file: " << entry.path());
        continue;
      }
      const auto &fileName = entry.path().filename().string();
      const auto &prefix = binaryLogPrefix_;
      if (prefix != fileName.substr(0, prefix.size())) {
        DLOG("Wrong prefix: " << entry.path());
        continue;
      }
      DLOG("Found: " << entry.path());
      std::ifstream input(entry.path().string(), std::fstream::binary);
      if (input.fail())
        PROFILER_RUNTIME_ERROR("Error opening file " << entry.path().string());
      _inputs.push_back(std::move(input));
    }
  }
  void run() {
    for (auto &input : _inputs) {
      const auto recordName = Serialize::decodeString(input);
      auto it = _funcMap.find(recordName);
      if (it == _funcMap.end())
        PROFILER_RUNTIME_ERROR(
            "Attempting to decode unknown record type: " << recordName);
      (it->second)(input, _out);
    }
  }

private:
  template <typename RecordTypes_>
  static std::unordered_map<std::string, DecodeFunc> genFuncMap() {
    std::unordered_map<std::string, Decoder::DecodeFunc> funcMap;
    auto addDecodeFunc = [&funcMap](auto dummy_) {
      using StorageType = typename decltype(dummy_)::Type;
      DLOG("Registering: " << typeid(StorageType).name());
      funcMap[typeid(StorageType).name()] = Internal::decodeStream<StorageType>;
    };
    Mpl::apply<Record::RecordStorageTypes>(addDecodeFunc);
    Mpl::apply<RecordTypes_>(addDecodeFunc);
    return funcMap;
  }

  std::unordered_map<std::string, DecodeFunc> _funcMap;
  std::ofstream _out;
  std::vector<std::ifstream> _inputs;
};

template <typename RecordList_> struct FileWriter {
  using RecordList = RecordList_;
  FileWriter() : FileWriter(".", ".cxxperf-log", "cxxperf-log.yaml") {}
  FileWriter(std::string binaryLogDir_, std::string binaryLogPrefix_,
             std::string yamlLogName_)
      : _binaryLogPrefix(std::move(binaryLogPrefix_)),
        _binaryLogDir(std::move(binaryLogDir_)),
        _yamlLogName(std::move(yamlLogName_)) {}
  template <typename RecordType_>
  void operator()(const RecordType_ & /*record_*/, std::size_t /*holderId_*/,
                  const std::string & /*userContext_*/) {
    DLOG("Saw record type " << typeid(RecordType_).name())
  }
  void finished() {
    auto decoder =
        Decoder<RecordList>(_binaryLogPrefix, _binaryLogDir, _yamlLogName);
  }

private:
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
  std::string _binaryLogPrefix;
  std::string _binaryLogDir;
  std::string _yamlLogName;
  std::unordered_map<std::type_index, std::ofstream> _outputs;
};
} // namespace Writer
} // namespace Profiler

#endif
