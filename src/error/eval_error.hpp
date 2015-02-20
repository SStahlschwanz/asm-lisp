#ifndef EVAL_ERROR_HPP_
#define EVAL_ERROR_HPP_

#include "../symbol.hpp"

#include <exception>

enum class eval_error
  : unsigned int
{
    EMPTY_STATEMENT,
    INVALID_COMMAND,
    DEF_TOO_FEW_ARGUMENTS,
    INVALID_DEFINED,
    ALREADY_DEFINED
};

constexpr const char* eval_error_strings[] =
{
    "empty statement is not allowed",
    "invalid command",
    "\"def\" requires at least 2 arguments",
    "invalid symbol defined: expected identifier",
    "name is already defined"
};

class eval_exception
  : public std::exception
{
public:
    eval_exception(symbol::source_type source, eval_error error)
      : source(std::move(source)),
        error(error)
    {}
    const char* what() const noexcept override
    {
        return eval_error_strings[(unsigned int)error];
    }

    symbol::source_type source;
    eval_error error;
};

#endif

