#ifndef RANGE_COMPARISON_HPP_
#define RANGE_COMPARISON_HPP_

#include "utils.hpp"
#include "tuple_utils.hpp"

#include <utility>

template<class LhsRange, class RhsRange>
bool operator==(LhsRange lhs, RhsRange rhs)
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

