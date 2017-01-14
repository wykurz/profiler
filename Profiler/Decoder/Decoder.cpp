#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Decoder/Decoder.h>
#include <Profiler/Exception/Exception.h>
#include <Profiler/Log/Log.h>
#include <Profiler/Record/Record.h>
#include <boost/filesystem.hpp>

namespace Profiler { namespace Decoder
{

    namespace fs = ::boost::filesystem;

namespace
{

    using NativeRecords = Mpl::TypeList<Record::TimeRecord>;

    template <typename RecordTypes_>
    auto genFuncMap()
    {
        std::unordered_map<std::string, Decoder::DecodeFunc> funcMap;
        auto addDecodeFunc = [&funcMap](auto dummy_) {
            using RecordType = typename decltype(dummy_)::Type;
            DLOG("Registering: " << typeid(RecordType).name());
            funcMap[typeid(RecordType).name()] = RecordType::decode;
        };
        Mpl::apply<NativeRecords>(addDecodeFunc);
        Mpl::apply<RecordTypes_>(addDecodeFunc);
        return funcMap;
    }

}

    Decoder::Decoder(const Config::Config& config_)
      : _funcMap(genFuncMap<Mpl::TypeList<> >())
    {
        fs::path logDir(config_.binaryLogDir);
        DLOG("Traversing: " << config_.binaryLogDir);
        for (auto& entry : fs::directory_iterator(logDir)) {
            if(!fs::is_regular_file(entry.status())) {
                DLOG("Not a file: " << entry.path());
                continue;
            }
            const auto& prefix = config_.binaryLogPrefix;
            if (prefix != entry.path().filename().string().substr(0, prefix.size())) {
                DLOG("Wrong prefix: " << entry.path());
                continue;
            }
            DLOG("Found: " << entry.path());
        }

        //_in(config_.binaryLogPrefix.c_str(), std::fstream::binary), _out()


        // if (_in.fail()) {
        //     std::stringstream ss;
        //     ss << "Error opening file " << config_.binaryLogPrefix.c_str();
        //     throw Exception::Runtime(ss.str());
        // }
        // registerRecordTypes<NativeRecords>();
    }

    void Decoder::run()
    {
        // std::size_t recordNameLength;
        // _in.read(reinterpret_cast<char*>(&recordNameLength), sizeof(recordNameLength));
        // std::cerr << "Reading log file length " << recordNameLength << "\n";
        // std::vector<char> bytes(recordNameLength);
        // _in.read(bytes.data(), recordNameLength);
        // const std::string recordName(bytes.data(), bytes.data() + recordNameLength);
        // auto it = _funcMap.find(recordName);
        // if (it == _funcMap.end()) {
        //     std::stringstream ss;
        //     ss << "Attempting to decode unknown record type: " << recordName;
        //     throw Exception::Runtime(ss.str());
        // }
        // (it->second)(_in, _out);
    }

}
}
