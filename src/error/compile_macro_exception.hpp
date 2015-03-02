#ifndef COMPILE_MACRO_EXCEPTION_HPP_
#define COMPILE_MACRO_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "core_exception.hpp"

namespace compile_macro_exception
{

struct compile_macro_exception
  : core_exception::core_exception
{
    using core_exception::core_exception::core_exception;
};

struct invalid_argument_number
  : compile_macro_exception
{
    invalid_argument_number()
      : compile_macro_exception{"compile_macro_invalid_argument_number"}
    {}
};
struct invalid_param_list
  : compile_macro_exception
{
    invalid_param_list(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_param_list", to_error_parameter(src)}
    {}
};
struct invalid_return_type
  : compile_macro_exception
{
    invalid_return_type(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_return_type", to_error_parameter(src)}
    {}
};
struct invalid_body
  : compile_macro_exception
{
    invalid_body(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_body", to_error_parameter(src)}
    {}
};
struct invalid_parameter_declaration
  : compile_macro_exception
{
    invalid_parameter_declaration(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_parameter_declaration", to_error_parameter(src)}
    {}
};
struct invalid_parameter_declaration_node_number
  : compile_macro_exception
{
    invalid_parameter_declaration_node_number(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_parameter_declaration_node_number", to_error_parameter(src)}
    {}
};
struct invalid_parameter_name
  : compile_macro_exception
{
    invalid_parameter_name(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_parameter_name", to_error_parameter(src)}
    {}
};
struct invalid_parameter_type
  : compile_macro_exception
{
    invalid_parameter_type(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_parameter_type", to_error_parameter(src)}
    {}
};
struct duplicate_parameter_name
  : compile_macro_exception
{
    duplicate_parameter_name(const symbol_source& src)
      : compile_macro_exception{"compile_macro_duplicate_parameter_name", to_error_parameter(src)}
    {}
};
struct invalid_block_list
  : compile_macro_exception
{
    invalid_block_list(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_block_list", to_error_parameter(src)}
    {}
};
struct empty_body
  : compile_macro_exception
{
    empty_body(const symbol_source& src)
      : compile_macro_exception{"compile_macro_empty_body", to_error_parameter(src)}
    {}
};
struct duplicate_variable_name
  : compile_macro_exception
{
    duplicate_variable_name(const symbol_source& src)
      : compile_macro_exception{"compile_macro_duplicate_variable_name", to_error_parameter(src)}
    {}
};
struct duplicate_block_name
  : compile_macro_exception
{
    duplicate_block_name(const symbol_source& src)
      : compile_macro_exception{"compile_macro_duplicate_block_name", to_error_parameter(src)}
    {}
};
struct invalid_block_definition
  : compile_macro_exception
{
    invalid_block_definition(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_block_definition", to_error_parameter(src)}
    {}
};
struct invalid_block_definition_argument_number
  : compile_macro_exception
{
    invalid_block_definition_argument_number(const symbol_source& src)
      : compile_macro_exception{"compile_macro_invalid_block_definition_argument_number", to_error_parameter(src)}
    {}
};

}

#endif

