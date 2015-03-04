#ifndef COMPILE_TYPE_ERROR_HPP_
#define COMPILE_TYPE_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace compile_type_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"int_invalid_argument_number", ""},
    {"int_invalid_argument_symbol", ""},
    {"int_invalid_argument_literal", ""},
    {"int_out_of_range_bit_width", ""}
};

constexpr std::size_t id(conststr str)
{
    return id(str, dictionary);
}

template<std::size_t error_id, class Location, class... ParamTypes>
[[ noreturn ]] void fatal(Location location, ParamTypes&&... params)
{
    static_assert(error_id < size(dictionary), "invalid error id");
    static_assert(error_id != std::numeric_limits<std::size_t>::max(), "invalid error id");
    throw compile_exception{error_kind::COMPILE_TYPE, error_id, location, std::forward<ParamTypes>(params)...};
}

}

#endif

