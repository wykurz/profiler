#ifndef _PROFILER_ALGORITHM_MPL_H
#define _PROFILER_ALGORITHM_MPL_H

namespace Profiler {
namespace Mpl {

template <typename... Args_> struct TypeList {};

template <typename T_> struct TypeInfo { using Type = T_; };

namespace Internal {

template <typename... Args_> struct Apply;

template <typename Arg_, typename... Args_>
struct Apply<TypeList<Arg_, Args_...>> {
  template <typename Func_> static void run(const Func_ &func_) {
    func_(TypeInfo<Arg_>());
    Apply<TypeList<Args_...>>::run(func_);
  }
};

template <> struct Apply<TypeList<>> {
  template <typename Func_> static void run(const Func_ & /*func_*/) {}
};
} // namespace Internal

template <typename Types_, typename Func_> void apply(const Func_ &func_) {
  Internal::Apply<Types_>::run(func_);
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

} // namespace Mpl
} // namespace Profiler

#endif
