#ifndef PERMUTED_HPP_
#define PERMUTED_HPP_

template<class RangeifiableIndices, class RangeifiableValues>
auto permuted(RangeifiableIndices&& indices_obj, RangeifiableValues&& values_obj)
{
    return mapped(indices_obj,
    [values_obj](const auto& index) -> decltype(auto)
    {
        return at(values_obj, index);
    });
}

#endif

