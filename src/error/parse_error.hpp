#ifndef PARSE_ERRORS_HPP_
#define PARSE_ERRORS_HPP_

#include "../symbol_source.hpp"

#include <exception>

enum class parse_error
  : unsigned int
{
    UNMATCHED_QUOTE,
    UNTERMINATED_SEMICOLON_LIST,
    UNMATCHED_CURLY_BRACE,
    UNMATCHED_SQUARE_BRACE,
    UNMATCHED_ROUND_BRACE,
    INVALID_CHARACTER
};

constexpr const char* parse_error_strings[] =
{
    "unmatched \"",
    "semicolon list not terminated with \";\"",
    "unmatched \"{\"",
    "unmatched \"[\"",
    "unmatched \"(\"",
    "invalid character"
};

class parse_exception
  : public std::exception
{
public:
    parse_exception(const file_position& pos, parse_error error)
      : pos(pos),
        error(error)
    {}
    const char* what() const noexcept override
    {
        return "parse exception";
    }

    file_position pos;
    parse_error error;
};

#endif

