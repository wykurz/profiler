#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Exception.h>
#include <Profiler/Log.h>
#include <Profiler/Record/Records.h>
#include <Profiler/Serialize.h>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Profiler {
namespace Writer {

namespace fs = boost::filesystem;

namespace Internal {

template <typename RecordType_> struct BinaryDecoder;

template <typename Clock_> struct BinaryDecoder<Record::ScopeStorage<Clock_>> {
  using Clock = Clock_;
  using Duration = typename Clock::Duration;
  static void run(std::istream &in_, std::ostream &out_) {
    auto name = Serialize::decodeString(in_);
    Duration t0;
    Duration t1;
    in_ >> t0 >> t1;
    auto depth = Serialize::decode<std::size_t>(in_);
    auto seqNum = Serialize::decode<std::size_t>(in_);
    out_ << "- seq: " << seqNum << "\n";
    out_ << "  name: \"" << name << "\"\n";
    out_ << "  t0: " << t0 << "\n";
    out_ << "  t1: " << t1 << "\n";
    out_ << "  depth: " << depth << "\n";
  }
};

template <typename Clock_> struct BinaryDecoder<Record::EventRecord<Clock_>> {
  using Clock = Clock_;
  using Duration = typename Clock::Duration;
  static void run(std::istream &in_, std::ostream &out_) {
    DLOG("Loop in ScopeRecordStart decode, currently at: " << in_.tellg());
    auto name = Serialize::decodeString(in_);
    Record::EventId<Clock> eventId;
    in_ >> eventId;
    Duration duration;
    in_ >> duration;
    out_ << "- name: " << name << "\n";
    out_ << "  event_id:\n";
    out_ << "    instance: " << eventId.instanceId << "\n";
    out_ << "    recorder: " << eventId.recorderId << "\n";
    out_ << "  time: " << duration << "\n";
  }
};

template <typename StorageType_>
inline void decodeStream(std::istream &in_, std::ostream &out_) {
  out_ << "records:\n";
  while (in_.good() && in_.peek() != EOF) {
    DLOG("Currently at: " << in_.tellg());
    BinaryDecoder<StorageType_>::run(in_, out_);
  }
}

template <typename RecordList_> struct Decoder {
  using RecordList = RecordList_;
  using DecodeFunc = std::function<void(std::istream &, std::ostream &)>;
  explicit Decoder(const std::string &yamlLogName_)
      : _funcMap(genFuncMap<RecordList>()),
        _out(yamlLogName_, std::fstream::trunc) {}
  void run(std::fstream &input_) {
    DLOG("Processing input file stream..." << input_.tellg());
    const auto recordName = Serialize::decodeString(input_);
    auto it = _funcMap.find(recordName);
    if (it == _funcMap.end())
      PROFILER_RUNTIME_ERROR(
          "Attempting to decode unknown record type: " << recordName);
    (it->second)(input_, _out);
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
};

template <typename RecordType_> struct BinaryEncoder;

template <typename Clock_> struct BinaryEncoder<Record::ScopeStorage<Clock_>> {
  using Record = Record::ScopeStorage<Clock_>;
  static void run(const Record &record_, std::ostream &out_) {
    Serialize::encodeString(out_, record_.name);
    out_ << record_.t0 << record_.t1;
    Serialize::encode(out_, record_.depth);
    Serialize::encode(out_, record_.seqNum);
  }
};

template <typename Clock_> struct BinaryEncoder<Record::EventRecord<Clock_>> {
  using Record = Record::EventRecord<Clock_>;
  static void run(const Record &record_, std::ostream &out_) {
    Serialize::encodeString(out_, record_.name);
    out_ << record_.eventId;
    out_ << record_.time;
  }
};
} // namespace Internal

template <typename RecordList_> struct FileWriter {
  using RecordList = RecordList_;
  FileWriter() : FileWriter(".", "cxxperf-log.yaml") {}
  FileWriter(std::string binaryLogDir_, std::string yamlLogName_)
      : _binaryLogDir(std::move(binaryLogDir_)),
        _yamlLogName(std::move(yamlLogName_)) {}
  // TODO(mateusz): Measure performance
  template <typename RecordType_>
  void operator()(const RecordType_ &record_, std::size_t /*holderId_*/,
                  const std::string & /*userContext_*/) {
    Internal::BinaryEncoder<RecordType_>::run(record_,
                                              getStream<RecordType_>());
  }
  void finish() {
    auto decoder = Internal::Decoder<RecordList>(_yamlLogName);
    auto it = _outputs.begin();
    while (it != _outputs.end()) {
      std::string fname = std::move(it->second.first);
      std::fstream &fstream = it->second.second;
      fstream.seekp(std::ios::beg);
      decoder.run(fstream);
      it = _outputs.erase(it);
      fs::remove(fname);
    }
  }

private:
  // TODO(mateusz): Is this slow?
  template <typename RecordType_> std::fstream &getStream() {
    auto it = _outputs.find(typeid(RecordType_));
    if (it != _outputs.end())
      return it->second.second;
    std::stringstream fname;
    fname << _binaryLogDir << "/.binlog_" << _idgen();
    DLOG("Creating new binary output file: " << fname.str());
    auto insPair = _outputs.emplace(
        std::piecewise_construct, std::forward_as_tuple(typeid(RecordType_)),
        std::forward_as_tuple(
            fname.str(),
            std::fstream(fname.str(), std::fstream::in | std::fstream::out |
                                          std::fstream::binary |
                                          std::fstream::trunc)));
    const std::string &recordTypeName = typeid(RecordType_).name();
    if (!insPair.second)
      PROFILER_RUNTIME_ERROR("Couldn't add a new output file for type "
                             << recordTypeName);
    auto &out = insPair.first->second.second;
    DLOG("Setup: " << recordTypeName.size() << " " << recordTypeName << " "
                   << std::size_t(&out));
    const std::size_t &nameSize = recordTypeName.size();
    out.write(reinterpret_cast<const char *>(&nameSize), sizeof(nameSize));
    out << recordTypeName;
    // RecordType_::encodePreamble(out, getManager().id());
    return out;
  }
  std::string _binaryLogDir;
  std::string _yamlLogName;
  std::unordered_map<std::type_index, std::pair<std::string, std::fstream>>
      _outputs;
  boost::uuids::random_generator _idgen;
};
} // namespace Writer
} // namespace Profiler

#endif
