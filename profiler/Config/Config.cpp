#include <Config/Config.h>

namespace Profiler { namespace Config
{

    Config::Config(std::string logFileName_)
        : logFileName(logFileName_)
    { }

    Config defaultConfig()
    {
        return Config(".cxxperf.log");
    }

    const Config& getConfig(const Config& config_)
    {
        static Config config(config_);
        return config;
    }

}
}
