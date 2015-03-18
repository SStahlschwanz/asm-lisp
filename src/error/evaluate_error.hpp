#ifndef EVALUATION_ERROR_HPP_
#define EVALUATION_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace evaluate_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"empty_top_level_statement", "empty top level statement is not allowed"},
    {"invalid_command", "invalid command"},
    {"def_invalid_argument_number", "too few arguments to def: expected at least 2"},
    {"invalid_defined_symbol", "invalid symbol to defined: expected identifier"},
    {"duplicate_definition", "duplicate definition"},
    {"not_a_macro", ""}
};

constexpr std::size_t id(conststr str)
{
    return index_of(str, dictionary);
}

template<std::size_t error_id, class Location, class... ParamTypes>
[[ noreturn ]] void fatal(Location location, ParamTypes&&... params)
{
    static_assert(error_id < size(dictionary), "invalid error id");
    static_assert(error_id != std::numeric_limits<std::size_t>::max(), "invalid error id");
    throw compile_exception{error_kind::EVALUATE, error_id, location, std::forward<ParamTypes>(params)...};
}

}

#endif

