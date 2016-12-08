#ifndef ALGORITHMS_MPL_H
#define ALGORITHMS_MPL_H

#include <utility>

namespace Profiler { namespace Mpl
{

    template <typename... Args> struct TypeList { };

    template <typename T> struct TypeInfo { using Type = T; };

namespace Internal
{

    template <typename... Args>
    struct Apply;

    template <typename Arg, typename... Args>
    struct Apply<TypeList<Arg, Args...> >
    {
        template <typename Func>
        static void run(const Func& func_)
        {
            func_(TypeInfo<Arg>());
            Apply<TypeList<Args...> >::run(func_);
        }

    };

    template <>
    struct Apply<TypeList<> >
    {
        template <typename Func>
        static void run(const Func& func_)
        { }
    };

}

    template <typename Types, typename Func>
    void apply(const Func& func_)
    {
        Internal::Apply<Types>::run(func_);
    }

}
}

#endif
