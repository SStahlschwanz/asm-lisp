#ifndef RANGE_HPP_
#define RANGE_HPP_

#include "zipped.hpp"
#include "mapped.hpp"
#include "tuple_utils.hpp"
#include "count.hpp"
#include "utils.hpp"
#include "save.hpp"
#include "comparison.hpp"

#include <cstddef>

template<class Range>
auto enumerate(Range r)
{
    return zipped(count(0), r);
}

template<class Iterator>
class iterator_range
{
  private:
    Iterator pos;
    Iterator end;
  public:
    iterator_range(Iterator begin, Iterator end)
    : pos(begin),
      end(end)
    {}
    
    typedef decltype(*pos) value_type;
    typedef std::size_t size_type;
    
    bool empty() const
    {
        return pos == end;
    }
    size_type length() const
    {
        return end - pos;
    }

    value_type front() const
    {
        return *pos;
    }
    void pop_front()
    {
        ++pos;
    }

    value_type back() const
    {
        return *(end - 1);
    }
    void pop_back()
    {
        --end;
    }

    void pop_front(size_type number)
    {
        pos += number;
    }
    void pop_back(size_type number)
    {
        end -= number;
    }
    void take_front(size_type number)
    {
        end = pos + number;
    }
    void take_back(size_type number)
    {
        pos = end - number;
    }
};

template<class Iterator>
iterator_range<Iterator> rangeify(Iterator begin, Iterator end)
{
    return iterator_range<Iterator>{begin, end};
}
template<class Container>
iterator_range<typename Container::iterator> rangeify(Container& container)
{
    return rangeify(container.begin(), container.end());
}
template<class Container>
iterator_range<typename Container::iterator> rangeify(const Container& container)
{
    return rangeify(container.begin(), container.end());
}

#endif

