#ifndef RANGE_COMPARISON_HPP_
#define RANGE_COMPARISON_HPP_

#include "utils.hpp"
#include "tuple_utils.hpp"

#include <utility>

namespace range_comparison_detail
{

template<class, class>
struct sfinae_helper
{
    typedef bool type;
};

}

template<class LhsRange, class RhsRange>
typename range_comparison_detail::sfinae_helper<decltype(std::declval<LhsRange>().front()), decltype(std::declval<RhsRange>().front())>::type operator==(LhsRange lhs, RhsRange rhs)
{
    bool result = true;
    for_each(zipped(lhs, rhs), unpacking([&](auto&& lhs_obj, auto&& rhs_obj)
    {
        result = result && (lhs_obj == rhs_obj);
    }));
    return result;
}

template<class LhsRange, class RhsRange>
bool operator!=(LhsRange lhs, RhsRange rhs)
{
    return !(std::move(lhs) == std::move(rhs));
}

#endif

