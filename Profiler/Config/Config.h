#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

#include <string>

namespace Profiler { namespace Config
{

    struct Config
    {
        Config(std::string binaryLogPrefix_ = ".cxxperf-log-", std::string binaryLogDir_ = ".",
               std::string yamlLogSuffix_ = ".yaml");
        bool operator!=(const Config& other_) const;
        const std::string binaryLogPrefix;
        const std::string binaryLogDir;
        const std::string yamlLogSuffix;
    };

    const Config& getConfig();
    const Config& setConfig(const Config& config_);

}
}

#endif
