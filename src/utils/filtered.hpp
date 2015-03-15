#ifndef FILTERED_HPP_
#define FILTERED_HPP_

#include "range_traits.hpp"

template<class Range, class Predicate>
class filtered_range
  : private Range
{
public:
    typedef range_tag range_tag;

    filtered_range(Range range, Predicate predicate)
      : Range(std::move(range)),
        predicate_(std::move(predicate))
    {
        pop_false_elements();
    }
    
    using Range::empty;
    using Range::front;

    void pop_front()
    {
        assert(!empty());
        Range::pop_front();
        pop_false_elements();
    }
private:
    Predicate predicate_;

    void pop_false_elements()
    {
        while(!empty() && !predicate_(front()))
            Range::pop_front();
    }
};

template<class Rangeifiable, class Predicate>
filtered_range<as_range<Rangeifiable>, Predicate> filtered(Rangeifiable&& obj, Predicate p)
{
    return {{std::forward<Rangeifiable>(obj)}, std::move(p)};
}

#endif

