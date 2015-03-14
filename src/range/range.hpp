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
#include <cassert>

template<class Range>
auto enumerate(Range r)
{
    return zipped(count(0), r);
}

template<class T>
class single_object_range
{
private:
    T* obj_;
public:
    typedef T& value_type;
    typedef std::size_t size_type;

    single_object_range(T& obj)
      : obj_(&obj)
    {}

    bool empty() const
    {
        return obj_ != nullptr;
    }
    size_type length() const
    {
        return static_cast<std::size_t>(obj_ != nullptr);
    }

    value_type front() const
    {
        assert(obj_);
        return *obj_;
    }
    value_type pop_front()
    {
        assert(obj_);
        obj_ = nullptr;
    }
};

template<class T>
single_object_range<T> static_range(T& obj)
{
    return {obj};
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
auto rangeify(Container& container)
{
    return rangeify(container.begin(), container.end());
}
template<class Container>
auto rangeify(const Container& container)
{
    return rangeify(container.begin(), container.end());
}

#endif

