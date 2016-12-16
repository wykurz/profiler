#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

#include <string>

namespace Profiler { namespace Config
{

    struct Config
    {
        Config(std::string logFileName_);
        bool operator!=(const Config& other_) const;
        const std::string logFileName;
    };

    const Config& getConfig();
    const Config& getConfig(const Config& config_);

}
}

#endif
