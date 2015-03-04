#ifndef EVALUATION_ERROR_HPP_
#define EVALUATION_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace evaluation_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"empty_top_level_statement", ""},
    {"invalid_command", ""},
    {"def_invalid_argument_number", ""},
    {"invalid_defined_symbol", ""},
    {"duplicate_definition", ""}
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
    throw compile_exception{error_kind::EVALUATION, error_id, location, std::forward<ParamTypes>(params)...};
}

}

#endif

