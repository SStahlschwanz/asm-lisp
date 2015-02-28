#ifndef PARSE_EXCEPTION_HPP_
#define PARSE_EXCEPTION_HPP_

#include "compile_exception.hpp"

namespace parse_exception
{

struct parse_exception
  : compile_exception
{
    using compile_exception::compile_exception;
};

struct unmatched_quote
  : parse_exception
{
    unmatched_quote(const code_location& first_quote_loc)
      : parse_exception{"parse_unmatched_quote", first_quote_loc}
    {}
};
struct unterminated_semicolon_list
  : parse_exception
{
    unterminated_semicolon_list(const code_location& supposed_end)
      : parse_exception{"parse_unterminated_semicolon_list", supposed_end}
    {}
};
struct unmatched_curly_brace
  : parse_exception
{
    unmatched_curly_brace(const code_location& opening_brace)
      : parse_exception{"parse_unmatched_curly_brace", opening_brace}
    {}
};
struct unmatched_square_brace
  : parse_exception
{
    unmatched_square_brace(const code_location& opening_brace)
      : parse_exception{"parse_unmatched_square_brace", opening_brace}
    {}
};
struct unmatched_round_brace
  : parse_exception
{
    unmatched_round_brace(const code_location& opening_brace)
      : parse_exception{"parse_unmatched_round_brace", opening_brace}
    {}
};
struct invalid_character
  : parse_exception
{
    invalid_character(const code_location& at, char c)
      : parse_exception{"pares_invalid_character", at, std::string{1, c}}
    {}
};

}

#endif

