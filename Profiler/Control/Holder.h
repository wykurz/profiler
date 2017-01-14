#ifndef CONTROL_HOLDER_H
#define CONTROL_HOLDER_H

#include <Profiler/Config/Config.h>
#include <Profiler/Control/RecordManager.h>
#include <atomic>
#include <memory>
#include <mutex>

namespace Profiler { namespace Control
{

    struct Output
    {
        using Ptr = std::unique_ptr<Output>;
        virtual ~Output() = default;
        virtual std::ostream& get() = 0;
        virtual void flush() = 0;
    };

    // TODO: redesign this to use unique_ptr with a custom deleter holding a reference to Holder

    struct Holder
    {
        using Ptr = std::unique_ptr<Holder, void(*)(Holder*)>;
        using Id = std::size_t;

        std::unique_lock<std::mutex> lock()
        {
            return std::unique_lock<std::mutex>(*_lock);
        }

        Ptr getPtr()
        {
            return Ptr(this, Holder::close);
        }

        bool isEmpty() const
        {
            return !_recordExtractor && !_finalExtractor.get();
        }

        void streamDirtyRecords()
        {
            if (getExtractor()) getExtractor()->streamDirtyRecords(_out->get());
        }

        void setup(RecordExtractor& recordExtractor_, std::unique_ptr<Output>&& out_)
        {
            PROFILER_ASSERT(!_recordExtractor && !_finalExtractor.get() && out_.get());
            _recordExtractor = &recordExtractor_;
            _out = std::move(out_);
            _recordExtractor->setupStream(_out->get());
        }

        void finalize()
        {
            _finalExtractor = _recordExtractor->moveToFinalExtractor();
            _recordExtractor = nullptr;
        }

        void flush()
        {
            if (_out) _out->flush();
        }

      private:
        static void close(Holder* holder_)
        {
            auto& finalExtractor = holder_->_finalExtractor;
            if (finalExtractor.get()) {
                finalExtractor.reset();
                holder_->_out.reset();
            }
        }

        RecordExtractor* getExtractor() const
        {
            if (_recordExtractor) return _recordExtractor;
            return _finalExtractor.get();
        }

        RecordExtractor* _recordExtractor = nullptr;
        std::unique_ptr<RecordExtractor> _finalExtractor;
        std::unique_ptr<Output> _out;
        std::unique_ptr<std::mutex> _lock = std::make_unique<std::mutex>();
    };

    struct Finalizer
    {
        Finalizer(Holder* holder_)
          : _holder(holder_)
        { }
        ~Finalizer()
        {
            if (_holder) {
                auto lk = _holder->lock();
                _holder->finalize();
            }
        }
      private:
        Holder* const _holder;
    };

    using HolderArray = std::vector<Holder>;

    struct OutputFactory
    {
        virtual Output::Ptr newOutput(Holder::Id extractorId_, Record::TypeId recordTypeId_) const = 0;
    };

    struct FileOutputs : OutputFactory
    {
        FileOutputs(const Config::Config& config_);
        virtual Output::Ptr newOutput(Holder::Id extractorId_, Record::TypeId recordTypeId_) const override;
      private:
        const Config::Config& _config;
    };

}
}

#endif
