#ifndef ALGORITHMS_MPL_H
#define ALGORITHMS_MPL_H

#include <utility>

namespace Profiler { namespace Mpl
{

    template <typename... Args_> struct TypeList { };

    template <typename T_> struct TypeInfo { using Type = T_; };

namespace Internal
{

    template <typename... Args_>
    struct Apply;

    template <typename Arg_, typename... Args_>
    struct Apply<TypeList<Arg_, Args_...> >
    {
        template <typename Func_>
        static void run(const Func_& func_)
        {
            func_(TypeInfo<Arg_>());
            Apply<TypeList<Args_...> >::run(func_);
        }

    };

    template <>
    struct Apply<TypeList<> >
    {
        template <typename Func_>
        static void run(const Func_& func_)
        { }
    };

}

    template <typename Types_, typename Func_>
    void apply(const Func_& func_)
    {
        Internal::Apply<Types_>::run(func_);
    }

}
}

#endif
