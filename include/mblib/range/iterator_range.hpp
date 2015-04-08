#ifndef MBLIB_RANGE_ITERATOR_RANGE_HPP_
#define MBLIB_RANGE_ITERATOR_RANGE_HPP_

#include <mblib/range/traits.hpp>
#include <cstddef>

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

#endif

