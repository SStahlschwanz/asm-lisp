#ifndef COMPILE_TYPE_ERROR_HPP_
#define COMPILE_TYPE_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace compile_type_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"int_invalid_argument_number", "invalid number of arguments to 'int': expected 1"},
    {"int_invalid_argument_symbol", "invalid argument: expected a literal (bit width)"},
    {"int_invalid_argument_literal", "invalid bit width: expected a positive integer"},
    {"int_out_of_range_bit_width", "invalid bit width: expected a positive integer"}
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

