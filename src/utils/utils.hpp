#ifndef RANGE_UTILS_HPP_
#define RANGE_UTILS_HPP_

#include "range_traits.hpp"

#include <cstddef>

template<class Rangeifiable, class Visitor>
void for_each(Rangeifiable&& obj, Visitor&& visitor)
{
    as_range<Rangeifiable> range(obj);
    while(!range.empty())
    {
        visitor(range.front());
        range.pop_front();
    }
}
template<class Range>
size_type_t<Range> length(const Range r)
{
    size_type_t<Range> result = 0;
    for_each(r, [&](auto&)
    {
        ++result;
    });
    return result;
}
template<class Range>
decltype(auto) at(Range r, size_type_t<Range> index)
{
    r.pop_front(index);
    return r.front();
}

#endif

