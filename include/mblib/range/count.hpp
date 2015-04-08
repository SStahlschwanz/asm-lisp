#ifndef MBLIB_RANGE_COUNT_HPP_
#define MBLIB_RANGE_COUNT_HPP_

#include <mblib/range/traits.hpp>

#include <cstddef>

class counting_range
{
public:
    typedef std::size_t value_type;
    typedef std::size_t size_type;
    typedef range_tag range_tag;
private:
    std::size_t i;
public:
    counting_range(size_t start)
      : i(start)
    {}

    bool empty() const
    {
        return false;
    }
    
    value_type front() const
    {
        return i;
    }
    void pop_front()
    {
        ++i;
    }

    void pop_front(std::size_t how_many)
    {
        i += how_many;
    }
};

inline counting_range count(std::size_t start)
{
    return {start};
}

#endif

