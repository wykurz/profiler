#ifndef CONTROL_HOLDER_H
#define CONTROL_HOLDER_H

#include <Profiler/Config/Config.h>
#include <Profiler/Control/RecordManager.h>
#include <Profiler/Decoder/Decoder.h>
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
        std::unique_lock<std::mutex> lock();
        bool isEmpty() const;
        void streamDirtyRecords();
        // TODO: There are 2x setup functions, which will be a source of errors.
        //       Use typesystem to handle initialization.
        void setOut(std::unique_ptr<Output>&& out_);
        void setRecordExtractor(RecordExtractor& recordExtractor_);
        /**
         * Usually called by Finalizer's destructor when a thread using the holder is shutting down. Can be called
         * manually, but care must be taken such that the thread which was using the holder before should not try to
         * write any more records, otherwise the resources used to hold those records will be lost.
         */
        void finalize();
        void flush();
      private:
        bool isFinalized() const;
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
        virtual ~OutputFactory() = default;
        virtual Output::Ptr newOutput(std::size_t extractorId_) const = 0;
    };

    struct FileOutputs : OutputFactory
    {
        FileOutputs(const Config::Config& config_);
        virtual Output::Ptr newOutput(std::size_t extractorId_) const override;
      private:
        const Config::Config& _config;
    };

}
}

#endif
