#include <Profiler/Control/Holder.h>
#include <Profiler/Log/Log.h>
#include <fstream>
#include <memory>

namespace Profiler { namespace Control
{

    FileOutputs::FileOutputs(const Config::Config& config_)
      : _config(config_)
    { }

namespace
{

    struct FileOut : Output
    {
        FileOut(const std::string& name_)
          : _out(name_, std::fstream::binary | std::fstream::trunc)
        {
            DLOG("FileOut " << name_ << " " << std::size_t(&_out));
        }
        virtual std::ostream& get() override
        {
            return _out;
        }
        virtual void flush() override
        {
            _out.flush();
        }
      private:
        std::ofstream _out;
    };

}

    Output::Ptr FileOutputs::newOutput(std::size_t extractorId_) const
    {
        return std::make_unique<FileOut>(_config.binaryLogPrefix + "." + std::to_string(extractorId_));
    }

}
}
