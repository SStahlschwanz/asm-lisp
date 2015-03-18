#ifndef ERROR_UTILS_HPP_
#define ERROR_UTILS_HPP_

#include "error_kind.hpp"

#include <boost/variant.hpp>

#include <utility>
#include <limits>
#include <vector>

class conststr
{
public:
    template<unsigned int N>
    constexpr conststr(const char (&arr)[N])
      : data_(arr),
        size(N - 1)
    {}

    constexpr const char* data() const
    {
        return data_;
    }

    friend constexpr bool operator==(const conststr& lhs, const conststr& rhs)
    {
        if(lhs.size != rhs.size)
            return false;
        for(std::size_t i = 0; i != lhs.size; ++i)
        {
            if(lhs.data_[i] != rhs.data_[i])
                return false;
        }
        return true;
    }
private:
    const char* data_;
    std::size_t size;
};

template<unsigned int N>
using error_dictionary = std::pair<conststr, conststr>[N];

template<unsigned int N>
constexpr bool has_duplicates(const error_dictionary<N>& dict)
{
    for(std::size_t i = 0; i != N; ++i)
    {
        for(std::size_t j = i + 1; j != N; ++j)
        {
            if(dict[i].first == dict[j].first)
                return true;
        }
    }
    return false;
}

template<unsigned int N>
constexpr std::size_t index_of(conststr str, const error_dictionary<N>& dict)
{
    for(std::size_t i = 0; i != N; ++i)
    {
        if(str == dict[i].first)
            return i;
    }
    return std::numeric_limits<std::size_t>::max();
}

template<unsigned int N>
constexpr std::size_t size(const error_dictionary<N>&)
{
    return N;
}

#endif

