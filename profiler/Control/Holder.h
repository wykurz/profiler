#ifndef CONTROL_HOLDER_H
#define CONTROL_HOLDER_H

#include <Control/RecordManager.h>
#include <atomic>
#include <memory>
#include <mutex>

namespace Profiler { namespace Control
{

    struct ExtractorWrapper
    {
        ExtractorWrapper(RecordExtractor* recordExtractor_)
          : _recordExtractor(recordExtractor_)
        { }
        ExtractorWrapper(std::unique_ptr<RecordExtractor> finalExtractor_)
          : _recordExtractor(nullptr),
            _finalExtractor(std::move(finalExtractor_))
        { }
        void streamDirtyRecords(std::ostream& out_)
        {
            if (get()) get()->streamDirtyRecords(out_);
        }
      private:
        RecordExtractor* get() const
        {
            if (_recordExtractor) return _recordExtractor;
            return _finalExtractor.get();
        }
        RecordExtractor* const _recordExtractor;
        std::unique_ptr<RecordExtractor> _finalExtractor;
    };

    struct Holder
    {
        std::unique_lock<std::mutex> lock()
        {
            return std::unique_lock<std::mutex>(*_lock);
        }
        bool isEmpty() const
        {
            return !_recordExtractor && !_finalExtractor.get();
        }
        ExtractorWrapper getExtractorWrapper()
        {
            if (_recordExtractor)
                return ExtractorWrapper(_recordExtractor);
            return  ExtractorWrapper(std::move(_finalExtractor));
        }
        void setRecordExtractor(RecordExtractor& recordExtractor_)
        {
            _recordExtractor = &recordExtractor_;
        }
        void finalize()
        {
            _finalExtractor = _recordExtractor->moveToFinalExtractor();
            _recordExtractor = nullptr;
        }
      private:
        RecordExtractor* _recordExtractor = nullptr;
        std::unique_ptr<RecordExtractor> _finalExtractor;
        std::unique_ptr<std::mutex> _lock = std::make_unique<std::mutex>();
    };

    using HolderArray = std::vector<Holder>;

}
}

#endif
