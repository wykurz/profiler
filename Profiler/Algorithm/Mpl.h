#ifndef _PROFILER_ALGORITHM_MPL_H
#define _PROFILER_ALGORITHM_MPL_H

#include <tuple>
#include <type_traits>

namespace Profiler {
namespace Mpl {

template <typename... Args_> struct TypeList {};

template <typename T_> struct TypeInfo { using Type = T_; };

namespace Internal {

template <typename... Args_> struct ApplyType;

template <typename Arg_, typename... Args_>
struct ApplyType<TypeList<Arg_, Args_...>> {
  template <typename Func_> static void run(const Func_ &func_) {
    func_(TypeInfo<Arg_>());
    ApplyType<TypeList<Args_...>>::run(func_);
  }
};

template <> struct ApplyType<TypeList<>> {
  template <typename Func_> static void run(const Func_ & /*func_*/) {}
};
} // namespace Internal

template <typename Types_, typename Func_> void apply(const Func_ &func_) {
  Internal::ApplyType<Types_>::run(func_);
}

template <typename Arg_, typename Types1_> struct Append;

template <typename Arg_, typename... Args_>
struct Append<Arg_, TypeList<Args_...>> {
  using type = TypeList<Args_..., Arg_>;
};

template <typename Types1_, typename Types2_, typename Res_ = TypeList<>>
struct Concat;

template <typename Arg_, typename... Args_, typename Types2_, typename Res_>
struct Concat<TypeList<Arg_, Args_...>, Types2_, Res_> {
  using type = typename Concat<TypeList<Args_...>, Types2_,
                               typename Append<Arg_, Res_>::type>::type;
};

template <typename Arg_, typename... Args_, typename Res_>
struct Concat<TypeList<>, TypeList<Arg_, Args_...>, Res_> {
  using type = typename Concat<TypeList<>, TypeList<Args_...>,
                               typename Append<Arg_, Res_>::type>::type;
};

template <typename Res_> struct Concat<TypeList<>, TypeList<>, Res_> {
  using type = Res_;
};

namespace Internal {

template <std::size_t Index_, typename... Args_> struct ApplyTuple {
  template <typename Func_>
  static void run(Func_ &&func_, std::tuple<Args_...> &args_) {
    func_(std::get<Index_>(args_));
    ApplyTuple<Index_ + 1, Args_...>::run(func_, args_);
  }
};

template <typename... Args_> struct ApplyTuple<sizeof...(Args_), Args_...> {
  template <typename Func_>
  static void run(Func_ && /*func_*/, std::tuple<Args_...> & /*args_*/) {}
};
} // namespace Internal

template <typename Func_, typename... Types_>
void apply(Func_ &&func_, std::tuple<Types_...> &args_) {
  Internal::ApplyTuple<0, Types_...>::run(func_, args_);
}
} // namespace Mpl
} // namespace Profiler

#endif
