#ifndef EVALUATION_EXCEPTION_HPP_
#define EVALUATION_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

namespace evaluation_exception
{

struct evaluation_exception
  : compile_exception
{
public:
    using compile_exception::compile_exception;
};

struct empty_top_level_statement
  : evaluation_exception
{
    empty_top_level_statement(const symbol_source& src)
      : evaluation_exception("evaluation_empty_top_level_statement", to_error_parameter(src))
    {}
};
struct invalid_command
  : evaluation_exception
{
    invalid_command(const symbol_source& src)
      : evaluation_exception("evaluation_invalid_command", to_error_parameter(src))
    {}
};
struct def_invalid_argument_number
  : evaluation_exception
{
    def_invalid_argument_number(const symbol_source& src)
      : evaluation_exception("evaluation_def_invalid_argument_number", to_error_parameter(src))
    {}
};
struct invalid_defined_symbol
  : evaluation_exception
{
    invalid_defined_symbol(const symbol_source& src)
      : evaluation_exception("evaluation_invalid_defined_symbol", to_error_parameter(src))
    {}
};
struct duplicate_definition
  : evaluation_exception
{
    duplicate_definition(const symbol_source& src)
      : evaluation_exception("evaluation_duplicate_definition", to_error_parameter(src))
    {}
};

}

#endif

