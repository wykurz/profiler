#ifndef _PROFILER_DECODER_DECODER_H
#define _PROFILER_DECODER_DECODER_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Algorithm/Stream.h>
#include <Profiler/Config.h>
#include <Profiler/Decoder.h>
#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <Profiler/Record/TimeScopeRecord.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace Profiler {

namespace fs = boost::filesystem;

struct Decoder {
  using DecodeFunc = std::function<void(std::istream &, std::ostream &)>;
  explicit Decoder(const Config &config_)
      : _funcMap(genFuncMap<Mpl::TypeList<>>()),
        _out(config_.yamlLogName, std::fstream::trunc) {
    const fs::path logDir(config_.binaryLogDir);
    DLOG("Traversing: " << logDir);
    for (auto &entry : fs::directory_iterator(logDir)) {
      if (!fs::is_regular_file(entry.status())) {
        DLOG("Not a file: " << entry.path());
        continue;
      }
      const auto &fileName = entry.path().filename().string();
      const auto &prefix = config_.binaryLogPrefix;
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
      const auto recordName = Algorithm::decodeString(input);
      auto it = _funcMap.find(recordName);
      if (it == _funcMap.end())
        PROFILER_RUNTIME_ERROR(
            "Attempting to decode unknown record type: " << recordName);
      (it->second)(input, _out);
    }
  }
  template <typename Record_> static void setupStream(std::ostream &out_) {
    const std::string &recordTypeName = typeid(Record_).name();
    DLOG("Setup: " << recordTypeName.size() << " " << recordTypeName << " "
                   << std::size_t(&out_))
    const std::size_t &nameSize = recordTypeName.size();
    out_.write(reinterpret_cast<const char *>(&nameSize), sizeof(nameSize));
    out_ << recordTypeName;
    Record_::preamble(out_);
  }

private:
  template <typename RecordTypes_>
  static std::unordered_map<std::string, DecodeFunc> genFuncMap() {
    std::unordered_map<std::string, Decoder::DecodeFunc> funcMap;
    auto addDecodeFunc = [&funcMap](auto dummy_) {
      using RecordType = typename decltype(dummy_)::Type;
      DLOG("Registering: " << typeid(RecordType).name());
      funcMap[typeid(RecordType).name()] = RecordType::decode;
    };
    Mpl::apply<Record::NativeRecords>(addDecodeFunc);
    Mpl::apply<RecordTypes_>(addDecodeFunc);
    return funcMap;
  }

  std::unordered_map<std::string, DecodeFunc> _funcMap;
  std::ofstream _out;
  std::vector<std::ifstream> _inputs;
};
} // namespace Profiler

#endif
