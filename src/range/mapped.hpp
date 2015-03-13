#ifndef MAPPED_HPP_
#define MAPPED_HPP_

#include <utility>

template <class Range, class Functor>
class mapped_range 
  : public Range
{
private:
    Functor functor;
public:
    typedef decltype(std::declval<Functor>()(std::declval<typename Range::value_type>())) value_type;
    typedef typename Range::size_type size_type;

    mapped_range(Range range, Functor functor)
      : Range(std::move(range)),
        functor(std::move(functor))
    {}

    value_type front() const
    {
        return functor(Range::front());
    }
    value_type back() const
    {
        return functor(Range::back());
    }
};

template<class RangeType, class FunctorType>
mapped_range<RangeType, FunctorType> mapped(RangeType range, FunctorType functor)
{
    return {std::move(range), std::move(functor)};
}

#endif

