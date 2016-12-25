#include <Profiler/Config/Config.h>
#include <Profiler/Exception/Exception.h>
#include <string>

namespace Profiler { namespace Config
{

    Config::Config(std::string binaryLogPrefix_, std::string yamlLogSuffix_)
      : binaryLogPrefix(binaryLogPrefix_),
        yamlLogSuffix(yamlLogSuffix_)
    { }

    bool Config::operator!=(const Config& other_) const
    {
        return binaryLogPrefix != other_.binaryLogPrefix or yamlLogSuffix != other_.yamlLogSuffix;
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

    const Config& setConfig(const Config& config_)
    {
        auto& config = doGetConfig(config_);
        if (config != config_) throw Exception::Runtime("Config appears to have been already set to a different value");
        return config;
    }

}
}
