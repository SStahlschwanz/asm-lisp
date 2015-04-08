#ifndef MBLIB_TUPLE_HPP_
#define MBLIB_TUPLE_HPP_

#include <tuple>
#include <utility>


namespace tuple_utils_detail
{

template<class Tuple>
using tuple_indices_t = std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>;

template<class F, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
{
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

template<class IntegerSequence>
struct integer_sequence_pop_front;
template<class T, T Head, T... Tail>
struct integer_sequence_pop_front<std::integer_sequence<T, Head, Tail...>>
{
    typedef std::integer_sequence<T, Tail...> type;
};

template<class IntegerSequence>
using integer_sequence_pop_front_t = typename integer_sequence_pop_front<IntegerSequence>::type;

}

template<class F, class Tuple>
constexpr decltype(auto) tuple_apply(F&& f, Tuple&& t)
{
    using namespace tuple_utils_detail;
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t), tuple_indices_t<Tuple>{});
}


template<class Functor>
struct unpacking_functor
{
    template<class Tuple>
    constexpr decltype(auto) operator()(Tuple&& t) const
    {
        return tuple_apply(f, std::forward<Tuple>(t));
    }

    Functor f;
};

template<class Functor>
unpacking_functor<Functor> unpacking(Functor&& f)
{
    return {std::forward<Functor>(f)};
}


namespace tuple_utils_detail
{

template<class Functor, class Tuple, std::size_t... Indices>
using decltype_tuple_t = std::tuple<decltype(std::declval<Functor>()(std::get<Indices>(std::declval<Tuple>())))...>;

template<class Functor, class Tuple, std::size_t... Indices>
constexpr auto map_impl(Tuple&& t, Functor&& f, std::index_sequence<Indices...>) -> decltype_tuple_t<decltype(f), decltype(t), Indices...>
{
    return decltype_tuple_t<decltype(f), decltype(t), Indices...>{f(std::get<Indices>(t))...};
}

}
// FIXME: doesn't work functors returning not by plain value (e.g. const ref, ref, rvalue ref)
template<class Functor, class Tuple>
constexpr auto tuple_map(Tuple&& t, Functor&& f)
{
    using namespace tuple_utils_detail;
    return map_impl(t, f, tuple_indices_t<Tuple>{});
}

namespace tuple_utils_detail
{

template<class Functor, class LastResult, class Tuple>
constexpr auto foldl_impl(LastResult&& last_result, Tuple&& t, Functor&& f, std::index_sequence<>)
{
    return std::forward<LastResult>(last_result);
}
template<class Functor, class LastResult, class Tuple, std::size_t HeadIndex, std::size_t... TailIndices>
constexpr decltype(auto) foldl_impl(LastResult&& last_result, Tuple&& t, Functor&& f, std::index_sequence<HeadIndex, TailIndices...>)
{
    return foldl_impl(f(last_result, std::get<HeadIndex>(t)), t, f, std::index_sequence<TailIndices...>{});
}

}

// FIXME: does this work for rvalue returns?
// FIXME: for an rvalue tuple argument, elements should be moved out of the tuple
template<class Initial, class Tuple, class Functor>
constexpr decltype(auto) tuple_foldl(Initial&& initial, Tuple&& t, Functor&& f)
{
    using namespace tuple_utils_detail;

    //typedef integer_sequence_pop_front_t<tuple_indices_t<Tuple>> next_indices;

    return foldl_impl(std::forward<Initial>(initial), t, f, tuple_indices_t<Tuple>{});
}

template<class Functor, class Tuple>
constexpr void tuple_for_each(Tuple&& t, Functor&& f)
{
    struct dummy_type
    {};
    tuple_foldl(dummy_type{}, t, [&](dummy_type dummy, auto&& obj)
    {
        f(std::forward<decltype(obj)>(obj));
        return dummy_type{};
    });
}


#endif

