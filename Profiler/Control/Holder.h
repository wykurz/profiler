#ifndef _PROFILER_CONTROL_HOLDER_H
#define _PROFILER_CONTROL_HOLDER_H

#include <Profiler/Config.h>
#include <Profiler/Control/RecordManager.h>
#include <Profiler/Decoder.h>
#include <atomic>
#include <memory>
#include <mutex>

namespace Profiler {
namespace Control {

struct Output {
  using Ptr = std::unique_ptr<Output>;
  virtual ~Output() = default;
  virtual std::ostream &get() = 0;
  virtual void flush() = 0;
};

// TODO(mateusz): redesign this to use unique_ptr with a custom deleter holding
// a reference to Holder.

struct Holder {
  using Ptr = std::unique_ptr<Holder, void (*)(Holder *)>;
  std::unique_lock<std::mutex> lock() {
    return std::unique_lock<std::mutex>(*_lock);
  }

  bool isEmpty() const {
    return (_recordExtractor == nullptr) && !_finalExtractor;
  }

  void streamDirtyRecords() {
    if (!isEmpty()) {
      PROFILER_ASSERT(_out.get());
      if (_recordExtractor != nullptr) {
        _recordExtractor->streamDirtyRecords(_out->get());
      } else {
        _finalExtractor->streamDirtyRecords(_out->get());
        _finalExtractor.reset();
        _out.reset();
      }
    }
  }

  // TODO(mateusz): There are 2x setup functions, which will be a source of
  // errors. Use typesystem to handle initialization.
  void setOut(std::unique_ptr<Output> &&out_) {
    PROFILER_ASSERT(!_recordExtractor);
    PROFILER_ASSERT(!_finalExtractor.get());
    PROFILER_ASSERT(out_.get());
    _out = std::move(out_);
  }

  void setRecordExtractor(RecordExtractor &recordExtractor_) {
    PROFILER_ASSERT(!_recordExtractor);
    PROFILER_ASSERT(!_finalExtractor.get());
    _recordExtractor = &recordExtractor_;
  }

  /**
   * Usually called by Finalizer's destructor when a thread using the holder is
   * shutting down. Can be called manually, but care must be taken such that the
   * thread which was using the holder should not try to write any more records,
   * otherwise the resources used to hold those records will be lost.
   */
  void finalize() {
    if (isEmpty() || isFinalized())
      return;
    _finalExtractor = _recordExtractor->moveToFinalExtractor();
    _recordExtractor = nullptr;
  }

  void flush() {
    if (_out)
      _out->flush();
  }

private:
  bool isFinalized() const {
    return (_recordExtractor == nullptr) && _finalExtractor;
  }

  RecordExtractor *_recordExtractor = nullptr;
  std::unique_ptr<RecordExtractor> _finalExtractor;
  std::unique_ptr<Output> _out;
  std::unique_ptr<std::mutex> _lock = std::make_unique<std::mutex>();
};

struct Finalizer {
  explicit Finalizer(Holder *holder_) : _holder(holder_) {}
  ~Finalizer() {
    if (_holder != nullptr) {
      auto lk = _holder->lock();
      _holder->finalize();
    }
  }

private:
  Holder *const _holder;
};

using HolderArray = std::vector<Holder>;

struct OutputFactory {
  virtual ~OutputFactory() = default;
  virtual Output::Ptr newOutput(std::size_t extractorId_) const = 0;
};

namespace Internal {

struct FileOut : Output {
  explicit FileOut(const std::string &name_)
      : _out(name_, std::fstream::binary | std::fstream::trunc) {
    DLOG("FileOut " << name_ << " " << std::size_t(&_out));
  }
  std::ostream &get() override { return _out; }
  void flush() override { _out.flush(); }

private:
  std::ofstream _out;
};
} // namespace Internal

struct FileOutputs : OutputFactory {
  explicit FileOutputs(const Config &config_) : _config(config_) {}
  Output::Ptr newOutput(std::size_t extractorId_) const override {
    return std::make_unique<Internal::FileOut>(_config.binaryLogPrefix + "." +
                                               std::to_string(extractorId_));
  }

private:
  const Config &_config;
};
} // namespace Control
} // namespace Profiler

#endif
