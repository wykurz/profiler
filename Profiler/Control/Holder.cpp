#include <Profiler/Control/Holder.h>
#include <Profiler/Log/Log.h>
#include <fstream>
#include <memory>

namespace Profiler { namespace Control
{

    std::unique_lock<std::mutex> Holder::lock()
    {
        return std::unique_lock<std::mutex>(*_lock);
    }

    bool Holder::isEmpty() const
    {
        return !_recordExtractor && !_finalExtractor;
    }

    void Holder::streamDirtyRecords()
    {
        if (!isEmpty()) {
            PROFILER_ASSERT(_out.get());
            if (_recordExtractor) {
                _recordExtractor->streamDirtyRecords(_out->get());
            }
            else {
                _finalExtractor.get()->streamDirtyRecords(_out->get());
                _finalExtractor.reset();
                _out.reset();
            }
        }
    }

    void Holder::setOut(std::unique_ptr<Output>&& out_)
    {
        PROFILER_ASSERT(!_recordExtractor);
        PROFILER_ASSERT(!_finalExtractor.get());
        PROFILER_ASSERT(out_.get());
        _out = std::move(out_);
    }

    void Holder::setRecordExtractor(RecordExtractor& recordExtractor_)
    {
        PROFILER_ASSERT(!_recordExtractor);
        PROFILER_ASSERT(!_finalExtractor.get());
        _recordExtractor = &recordExtractor_;
    }

    void Holder::finalize()
    {
        if (isEmpty() || isFinalized()) return;
        _finalExtractor = _recordExtractor->moveToFinalExtractor();
        _recordExtractor = nullptr;
    }

    void Holder::flush()
    {
        if (_out) _out->flush();
    }

    bool Holder::isFinalized() const
    {
        return !_recordExtractor && _finalExtractor;
    }

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
