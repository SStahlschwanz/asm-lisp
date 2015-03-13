#ifndef RANGE_UTILS_HPP_
#define RANGE_UTILS_HPP_

#include <cstddef>

template<class Range, class Visitor>
void for_each(Range range, Visitor visitor)
{
    while(!range.empty())
    {
        visitor(range.front());
        range.pop_front();
    }
}
template<class Range>
std::size_t length(const Range r)
{
    typename Range::size_type result = 0;
    for_each(r, [&](auto&)
    {
        ++result;
    });
    return result;
}
template<class Range>
typename Range::value_type at(Range r, typename Range::size_type i)
{
    r.pop_front(i);
    return r.front();
}


#endif

