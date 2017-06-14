#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <Profiler/Log.h>
#include <typeinfo>

namespace Profiler {
namespace Writer {

struct FileWriter {
  // TODO(mateusz): writer's must also get the ID of the holder
  template <typename RecordType_>
  void operator()(const RecordType_ & /*record_*/, std::size_t /*holderId_*/,
                  const std::string & /*userContext_*/) {
    DLOG("Saw record type " << typeid(RecordType_).name())
  }
};

// struct OutputFactory {
//   virtual ~OutputFactory() = default;
//   virtual Output::Ptr newOutput(std::size_t extractorId_) const = 0;
// };

// namespace Internal {

// struct FileOut : Output {
//   explicit FileOut(const std::string &name_)
//       : _out(name_, std::fstream::binary | std::fstream::trunc) {
//     DLOG("FileOut " << name_ << " " << std::size_t(&_out));
//   }
//   std::ostream &get() override { return _out; }
//   void flush() override { _out.flush(); }

// private:
//   std::ofstream _out;
// };
// } // namespace Internal

// struct FileOutputs : OutputFactory {
//   explicit FileOutputs(const Config &config_) : _config(config_) {}
//   Output::Ptr newOutput(std::size_t extractorId_) const override {
//     return std::make_unique<Internal::FileOut>(_config.binaryLogPrefix + "."
//     +
//                                                std::to_string(extractorId_));
//   }

// private:
//   const Config &_config;
// };

// template <typename RecordType_> static void setupStream(std::ostream &out_) {
//   const std::string &recordTypeName = typeid(RecordType_).name();
//   DLOG("Setup: " << recordTypeName.size() << " " << recordTypeName << " "
//                  << std::size_t(&out_))
//   const std::size_t &nameSize = recordTypeName.size();
//   out_.write(reinterpret_cast<const char *>(&nameSize), sizeof(nameSize));
//   out_ << recordTypeName;
//   RecordType_::encodePreamble(out_, getManager().id());
// }

} // namespace Writer
} // namespace Profiler

#endif
