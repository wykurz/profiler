#ifndef DECODER_DECODER_H
#define DECODER_DECODER_H

#include <Profiler/Config/Config.h>
#include <Profiler/Record/Record.h>
#include <Profiler/Log/Log.h>
#include <functional>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace Profiler { namespace Decoder
{

    inline void setupStream(std::ostream& out_, Record::TypeId recordType_)
    {
        const std::string& recordTypeName = recordType_.name();
        DLOG("Setup: " << recordTypeName.size() << " " << recordTypeName << " " << std::size_t(&out_))
        const std::size_t& nameSize = recordTypeName.size();
        out_.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
        out_ << recordTypeName;
    }

    struct Decoder
    {
        using DecodeFunc = std::function<void(std::istream&, std::ostream&)>;
        Decoder(const Config::Config& config_);
        void run();
      private:
        std::unordered_map<std::string, DecodeFunc> _funcMap;
        std::ofstream _out;
        std::vector<std::ifstream> _inputs;
    };

}
}

#endif
