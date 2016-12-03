#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

#include <string>

namespace Profiler { namespace Config
{

    struct Config
    {
        Config(std::string logFileName_);

        const std::string logFileName;
    };

    Config defaultConfig();

    const Config& getConfig(const Config& config_ = defaultConfig());

}
}

#endif
