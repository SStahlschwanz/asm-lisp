#ifndef MBLIB_RANGE_ZIPPED_RANGE_HPP_
#define MBLIB_RANGE_ZIPPED_RANGE_HPP_

#include <mblib/tuple.hpp>
#include <mblib/range/traits.hpp>
#include <mblib/range/count.hpp>

#include <utility>
#include <tuple>
#include <cassert>


template<class... RangeTypes>
class zipped_range
{
public:
    typedef range_tag range_tag;
    typedef std::tuple<value_type_t<RangeTypes>...> value_type;
private:
    std::tuple<RangeTypes...> ranges_;
public:
    zipped_range(std::tuple<RangeTypes...> ranges)
      : ranges_(std::move(ranges))
    {}

    bool empty() const
    {
        return tuple_foldl(false, ranges_, [](bool previous_result, const auto& range)
        {
            return previous_result || range.empty();
        });
    }
    value_type front() const
    {
        assert(!empty());
        return tuple_map(ranges_, [](const auto& r) -> decltype(r.front())
        {
            return r.front();
        });
    }
    void pop_front()
    {
        assert(!empty());
        tuple_for_each(ranges_, [&](auto& r)
        {
            r.pop_front();
        });
    }
};

template<class... Rangeifiables>
zipped_range<as_range<Rangeifiables>...> zipped(Rangeifiables&&... objs)
{
    return {std::make_tuple(as_range<Rangeifiables>{objs}...)};
}

template<class... Rangeifiables>
auto enumerate(Rangeifiables&&... objs)
{
    return zipped(count(0), std::forward<Rangeifiables>(objs)...);
}

#endif

