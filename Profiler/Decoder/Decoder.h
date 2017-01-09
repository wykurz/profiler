#ifndef DECODER_DECODER_H
#define DECODER_DECODER_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Config/Config.h>
#include <Profiler/Exception/Exception.h>
#include <Profiler/Record/Record.h>
#include <functional>
#include <fstream>
#include <iostream>
#include <typeindex>
#include <unordered_map>

namespace Profiler { namespace Decoder
{

namespace Internal
{

    template <typename OutRecord_>
    void convert(std::istream& in_, std::ostream& out_)
    {
        OutRecord_ record;
        while (in_.eof()) in_ >> record >> out_;
    }

}

    struct Decoder
    {
        using NativeRecords = Mpl::TypeList<Record::TimeRecord>;

        Decoder(const Config::Config& config_)
            : _in(config_.binaryLogPrefix.c_str()), _out()
        {
            registerRecordTypes<NativeRecords>();
        }

        template <typename RecordTypes_>
        void registerRecordTypes()
        {
            Mpl::apply<RecordTypes_>([this](auto dummy_) {
                    this->_funcMap[typeid(dummy_).name()] = decltype(dummy_)::Type::decode;
                });
        }

        void run()
        {
            std::string record;
            _in >> record;
            auto it = _funcMap.find(record);
            if (it == _funcMap.end()) throw Exception::Runtime("Attempting to decode unknown record type.");
            (it->second)(_in, _out);
        }

      private:
        using DecodeFunc = std::function<void(std::istream&, std::ostream&)>;
        std::ifstream _in;
        std::ofstream _out;
        std::unordered_map<std::string, DecodeFunc> _funcMap;
    };

}
}

#endif
