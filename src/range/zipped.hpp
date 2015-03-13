#ifndef ZIPPED_RANGE_HPP_
#define ZIPPED_RANGE_HPP_

#include <utility>


template<class FirstRange, class SecondRange>
class zipped_range
{
public:
    typedef std::pair<typename FirstRange::value_type, typename SecondRange::value_type> value_type;
    typedef std::size_t size_type;
private:
    std::pair<FirstRange, SecondRange> ranges_;
public:
    zipped_range(std::pair<FirstRange, SecondRange> ranges)
      : ranges_(std::move(ranges))
    {}

    bool empty() const
    {
        return ranges_.first.empty() || ranges_.second.empty();
    }
    value_type front() const
    {
        return value_type(ranges_.first.front(), ranges_.second.front());
    }
    void pop_front()
    {
        ranges_.first.pop_front();
        ranges_.second.pop_front();
    }
    void pop_front(size_type i)
    {
        ranges_.first.pop_front(i);
        ranges_.second.pop_front(i);
    }
    void take_front(size_type i)
    {
        ranges_.first.take_front(i);
        ranges_.second.take_front(i);
    }
};

template <class First, class Second>
zipped_range<First, Second> zipped(First f, Second s)
{
    return std::make_pair(std::move(f), std::move(s));
}

#endif

