#ifndef DECODER_DECODER_H
#define DECODER_DECODER_H

#include <Profiler/Config/Config.h>
#include <functional>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace Profiler { namespace Decoder
{

    struct Decoder
    {
        using DecodeFunc = std::function<void(std::istream&, std::ostream&)>;
        Decoder(const Config::Config& config_);
        void run();
      private:
        std::vector<std::ifstream> _inputs;
        std::ofstream _out;
        std::unordered_map<std::string, DecodeFunc> _funcMap;
    };

}
}

#endif
