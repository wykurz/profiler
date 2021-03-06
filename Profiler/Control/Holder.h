#ifndef _PROFILER_CONTROL_HOLDER_H
#define _PROFILER_CONTROL_HOLDER_H

#include <Profiler/Algorithm/Mpl.h>
#include <Profiler/Config.h>
#include <Profiler/Control/RecordManager.h>
#include <Profiler/Log.h>
#include <atomic>
#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <fstream>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Profiler {
namespace Control {

template <typename RecordType_> struct Holder {
  using RecordType = RecordType_;
  using RecordManager = Control::RecordManager<RecordType>;
  using DirtyRecordsIter = Control::DirtyRecordsIter<RecordType>;
  explicit Holder(std::mutex &lock_, std::size_t holderId_,
                  std::string userContext_)
      : _lockPtr(&lock_), _holderId(holderId_),
        _userContext(std::move(userContext_)) {}
  void setRecordManager(RecordManager &recordManager_) {
    _recordManagerPtr = &recordManager_;
  }
  Control::DirtyRecordsIter<RecordType> getDirtyRecords() {
    if (_recordManagerPtr)
      return _recordManagerPtr->getDirtyRecords();
    return std::move(_dirtyRecords);
  }
  void finalize() {
    if (!_recordManagerPtr)
      return;
    PROFILER_ASSERT(_dirtyRecords.next() == nullptr);
    _dirtyRecords = _recordManagerPtr->getFinalRecords();
    _recordManagerPtr = nullptr;
  }
  std::unique_lock<std::mutex> acquireLock() {
    return std::unique_lock<std::mutex>(*_lockPtr);
  }
  std::unique_lock<std::mutex> adoptLock() {
    DLOG("Adopting lock for holder, lock ptr: " << _lockPtr);
    return std::unique_lock<std::mutex>(*_lockPtr, std::adopt_lock);
  }
  std::size_t getId() const { return _holderId; }
  const std::string &getUserContext() const { return _userContext; }

private:
  std::mutex *_lockPtr;
  std::size_t _holderId;
  std::string _userContext;
  // TODO(mateusz): We could use a variant here as well.
  RecordManager *_recordManagerPtr = nullptr;
  DirtyRecordsIter _dirtyRecords;
};

template <typename... RecordList_> struct HolderVariant {
  struct Empty {};
  using VariantType = boost::variant<Empty, Holder<RecordList_>...>;
  void *reserve(std::type_index type_, std::size_t holderId_,
                const std::string &userContext_) {
    DLOG("Reserving variant, lock ptr: " << &_lock);
    _lock.lock();
    if (_variant.which() != 0) {
      _lock.unlock();
      return nullptr;
    }
    return initializer().set(_variant, type_, holderId_, userContext_, _lock);
  }
  template <typename VisitorFunc_> void apply(VisitorFunc_ &func_) {
    VisitorWrapper<VisitorFunc_> wrapper(func_);
    std::unique_lock<std::mutex> ulock(_lock);
    _variant.apply_visitor(wrapper);
  }

private:
  struct Initializer {
    using FuncMap = std::unordered_map<
        std::type_index,
        std::function<void *(VariantType &, std::size_t holderId_,
                             const std::string &userContext_, std::mutex &)>>;
    Initializer() {
      Mpl::apply<Mpl::TypeList<RecordList_...>>([this](auto dummy_) {
        using RecordType = typename decltype(dummy_)::Type;
        DLOG("Defining init function for record type "
             << typeid(RecordType).name());
        auto func = [this](VariantType &variant_, std::size_t holderId_,
                           const std::string &userContext_, std::mutex &lock_) {
          variant_ = Holder<RecordType>(lock_, holderId_, userContext_);
          return static_cast<void *>(boost::get<Holder<RecordType>>(&variant_));
        };
        this->_funcmap[typeid(RecordType)] = func;
      });
    }
    void *set(VariantType &variant_, std::type_index type_,
              std::size_t holderId_, const std::string &userContext_,
              std::mutex &lock_) const {
      DLOG("Setting variant of type " << type_.name());
      auto it = _funcmap.find(type_);
      PROFILER_ASSERT(it != _funcmap.end());
      return it->second(variant_, holderId_, userContext_, lock_);
    }

  private:
    FuncMap _funcmap;
  };
  static const Initializer &initializer() {
    static Initializer instance;
    return instance;
  }
  template <typename VisitorFunc_>
  struct VisitorWrapper : boost::static_visitor<> {
    explicit VisitorWrapper(VisitorFunc_ &func_) : _func(func_) {}
    void operator()(Empty /*unused*/) {}
    template <typename HolderType_> void operator()(HolderType_ &holder_) {
      _func(holder_);
    }

  private:
    VisitorFunc_ &_func;
  };

  mutable std::mutex _lock;
  VariantType _variant{Empty()};
};

template <typename RecordType_> struct Finalizer {
  using HolderType = Holder<RecordType_>;
  using FinalizerFunc =
      std::function<void(Control::DirtyRecordsIter<RecordType_> &&)>;
  explicit Finalizer(HolderType *holder_) : _holder(holder_) {}
  ~Finalizer() {
    if (_holder) {
      auto lock = _holder->acquireLock();
      _holder->finalize();
    }
  }

private:
  HolderType *_holder;
};

template <typename... RecordList_> struct HolderArray;

template <typename... RecordList_>
struct HolderArray<Mpl::TypeList<RecordList_...>> {
  static constexpr std::size_t MaxThreads = 1024;
  // Map RecordList_ types into std::type_index hash according to the variant
  // index.
  // TODO(mateusz): This must be thread safe
  void *findHolder(std::type_index type_, std::size_t holderId_,
                   const std::string &userContext_) {
    DLOG("Search for new holder for type " << type_.name());
    // TODO(mateusz): add per-thread offset based on where was the last found
    // holder
    int index = 0;
    int count = MaxThreads;
    while (0 < count--) {
      auto &variant = _array[index++ % _array.size()];
      auto res = variant.reserve(type_, holderId_, userContext_);
      if (res)
        return res;
    }
    return nullptr;
  }

  template <typename VisitorFunc_> void applyAll(VisitorFunc_ func_) {
    for (auto &variant : _array)
      variant.apply(func_);
  }

private:
  std::array<HolderVariant<RecordList_...>, MaxThreads> _array;
};

} // namespace Control
} // namespace Profiler

#endif
