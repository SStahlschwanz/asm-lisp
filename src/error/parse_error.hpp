#ifndef PARSE_ERROR_HPP_
#define PARSE_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace parse_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"unmatched_quote", "unmatched '\"'"},
    {"unterminated_semicolon_list", "expected ';'"},
    {"unmatched_curly_brace", "unmatched '{'"},
    {"unmatched_square_brace", "unmatched '['"},
    {"unmatched_round_brace", "unmatched '('"},
    {"invalid_character", "unexpected character"}
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
    throw compile_exception{error_kind::PARSE, error_id, location, std::forward<ParamTypes>(params)...};
}

}

#endif

