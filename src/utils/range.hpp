#ifndef RANGE_HPP_
#define RANGE_HPP_

#include "zipped.hpp"
#include "mapped.hpp"
#include "tuple_utils.hpp"
#include "count.hpp"
#include "utils.hpp"
#include "save.hpp"
#include "comparison.hpp"
#include "range_traits.hpp"
#include "filtered.hpp"

#include <cstddef>
#include <cassert>

template<class... Rangeifiables>
auto enumerate(Rangeifiables&&... objs)
{
    return zipped(count(0), std::forward<Rangeifiables>(objs)...);
}

template<class T>
class single_object_range
{
private:
    T* obj_;
public:
    typedef range_tag range_tag;

    single_object_range(T& obj)
      : obj_(&obj)
    {}

    bool empty() const
    {
        return obj_ == nullptr;
    }
    std::size_t length() const
    {
        return static_cast<std::size_t>(obj_ != nullptr);
    }

    T& front() const
    {
        assert(obj_);
        return *obj_;
    }
    void pop_front()
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

    template<class T>
    using is_this_type = std::is_same<std::decay_t<T>, iterator_range<Iterator>>;
    typedef decltype(*pos) value_type;

public:
    template<class Container>
    iterator_range(Container&& c, typename std::enable_if<!is_this_type<Container>{}, void*>::type = nullptr)
      : pos(std::begin(c)),
        end(std::end(c))
    {}
    iterator_range(Iterator begin, Iterator end)
      : pos(begin),
        end(end)
    {}
    
    typedef range_tag range_tag;
    
    bool empty() const
    {
        return pos == end;
    }
    std::size_t length() const
    {
        return end - pos;
    }

    value_type front() const
    {
        assert(!empty());
        return *pos;
    }
    void pop_front()
    {
        assert(!empty());
        ++pos;
    }

    value_type back() const
    {
        assert(!empty());
        return *(end - 1);
    }
    void pop_back()
    {
        assert(!empty());
        --end;
    }

    void pop_front(std::size_t number)
    {
        assert(!empty());
        pos += number;
    }
    void pop_back(std::size_t number)
    {
        assert(!empty());
        end -= number;
    }
    void take_front(std::size_t number)
    {
        assert(!empty());
        end = pos + number;
    }
    void take_back(std::size_t number)
    {
        assert(!empty());
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

