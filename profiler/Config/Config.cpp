#include <Config/Config.h>
#include <Exception/Exception.h>
#include <string>

namespace Profiler { namespace Config
{

    Config::Config(std::string logFileName_)
        : logFileName(logFileName_)
    { }

    bool Config::operator!=(const Config& other_) const
    {
        return logFileName != other_.logFileName;
    }

namespace
{

    Config defaultConfig()
    {
        return Config(".cxxperf.log");
    }

    const Config& doGetConfig(const Config& config_ = defaultConfig())
    {
        static Config config(config_);
        return config;
    }

}

    const Config& getConfig()
    {
        return doGetConfig();
    }

    const Config& getConfig(const Config& config_)
    {
        auto& config = doGetConfig(config_);
        if (config != config_) throw Exception::Runtime("Config appears to have been already set to a different value");
        return config;
    }

}
}
