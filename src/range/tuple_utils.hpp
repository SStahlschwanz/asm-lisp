#ifndef TUPLE_UTILS_HPP_
#define TUPLE_UTILS_HPP_

#include <tuple>
#include <utility>


template<class FunctorType>
struct unpacking_functor
{
    template<class First, class Second>
    auto operator()(std::pair<First, Second> p)
    {
        return functor(p.first, p.second);
    }

    FunctorType functor;
};

template<class FunctorType>
unpacking_functor<FunctorType> unpacking(FunctorType functor)
{
    return unpacking_functor<FunctorType>{std::move(functor)};
}

#endif

