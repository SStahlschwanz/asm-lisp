#ifndef FUNCTOR_UTILS_HPP_
#define FUNCTOR_UTILS_HPP_

#include <utility>

namespace boost
{

template<class ResultType>
class static_visitor;

};


namespace functor_utils_detail
{
struct dummy_type
{};
};

template<class ReturnType, class... Functors>
struct overloaded_functor;

template<class ReturnType>
struct overloaded_functor<ReturnType>
  : boost::static_visitor<ReturnType>
{
    ReturnType operator()(const functor_utils_detail::dummy_type)
    {
        return std::declval<ReturnType>();
    };
};

template<class ReturnType, class HeadFunctor, class... TailFunctors>
struct overloaded_functor<ReturnType, HeadFunctor, TailFunctors...>
  : HeadFunctor,
    overloaded_functor<ReturnType, TailFunctors...>
{
    using HeadFunctor::operator();
    using overloaded_functor<ReturnType, TailFunctors...>::operator();

    overloaded_functor(HeadFunctor head_functor, TailFunctors... tail_functors)
      : HeadFunctor(std::move(head_functor)),
        overloaded_functor<ReturnType, TailFunctors...>(std::move(tail_functors)...)
    {}
};

template<class ReturnType, class... Functors>
overloaded_functor<ReturnType, Functors...> overloaded(Functors... functors)
{
    return {std::move(functors)...};
}

#endif

