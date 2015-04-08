#ifndef MBLIB_RANGE_MAPPED_HPP_
#define MBLIB_RANGE_MAPPED_HPP_

#include <utility>

#include <mblib/range/traits.hpp>
#include <mblib/range/simple_algorithms.hpp>

template <class Range, class Functor>
class mapped_range 
  : public Range
{
private:
    Functor functor;
    typedef decltype(functor(std::declval<value_type_t<Range>>())) value_type;
public:
    typedef range_tag range_tag;

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

template<class Rangeifiable, class FunctorType>
mapped_range<as_range<Rangeifiable>, FunctorType> mapped(Rangeifiable&& obj, FunctorType&& functor)
{
    return {as_range<Rangeifiable>{std::forward<Rangeifiable>(obj)}, std::forward<FunctorType>(functor)};
}

template<class RangeifiableValues, class RangeifiableIndices>
auto permuted(RangeifiableValues&& values_obj, RangeifiableIndices&& indices_obj)
{
    as_range<RangeifiableValues> values_range(values_obj);
    return mapped(indices_obj,
    [values_range](const auto& index) -> decltype(auto)
    {
        return at(values_range, index);
    });
}

#endif

