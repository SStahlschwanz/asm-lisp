#ifndef COMPILE_MACRO_EXCEPTION_HPP_
#define COMPILE_MACRO_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

class macro_compilation_exception
  : public compile_exception
{
public:
    template<class... Types>
    macro_compilation_exception(Types&&... types)
    {}
};

struct macro_invalid_argument_number
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct macro_invalid_param_list
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct identifier_not_defined
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_return_type
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_body
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_macro_parameter_declaration
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_parameter_declaration_node_number
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_parameter_name
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_parameter_type
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct duplicate_parameter_name
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_block_list
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct macro_empty_body
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct duplicate_variable_name
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct duplicate_block_name
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_block_definition
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};
struct invalid_block_definiton_argument_number
  : macro_compilation_exception
{
    using macro_compilation_exception::macro_compilation_exception;
};

#endif

