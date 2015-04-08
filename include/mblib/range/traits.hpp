#ifndef MBLIB_RANGE_TRAITS_HPP_
#define MBLIB_RANGE_TRAITS_HPP_

#include <utility>

struct range_tag
{};

template<class Iterator>
class iterator_range;

namespace range_traits_detail
{

template<class T>
struct is_range_impl
{
    typedef char no;
    static_assert(sizeof(no) == 1, "");

    template<class S>
    static typename S::range_tag (&test(void*))[2];
    template<class S>
    static no test(...);

    static constexpr bool value = sizeof(test<T>(0)) != sizeof(no);
};

template<class T, bool IsRange>
struct range_type_selector;

template<class T>
struct range_type_selector<T, true>
{
    typedef std::decay_t<T> type;
};
template<class T>
struct range_type_selector<T, false>
{
private:
    typedef decltype(std::declval<T>().begin()) iterator_type;
public:
    typedef iterator_range<iterator_type> type;
};

}

template<class T>
using is_range_t = std::integral_constant<bool, range_traits_detail::is_range_impl<T>::value>;

template<class T>
using as_range = typename range_traits_detail::range_type_selector<T, is_range_t<std::remove_reference_t<T>>{}>::type;


template<class T>
struct value_type
{
    static_assert(is_range_t<T>{}, "value_type: T is not a range");
    typedef decltype(std::declval<T>().front()) type;
};
template<class T>
using value_type_t = typename value_type<T>::type;

template<class T>
struct size_type
{
    static_assert(is_range_t<T>{}, "size_type: T is not a range");
    typedef std::size_t type;
};
template<class T>
using size_type_t = typename size_type<T>::type;

#endif

