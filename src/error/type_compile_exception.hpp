#ifndef TYPE_COMPILE_EXCEPTION_HPP_
#define TYPE_COMPILE_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

namespace type_compile_exception
{

struct type_compile_exception
  : compile_exception
{
    using compile_exception::compile_exception;
};

struct int_invalid_argument_number
  : type_compile_exception
{
    int_invalid_argument_number()
      : type_compile_exception("type_compile_int_invalid_argument_number")
    {}
};
struct int_invalid_argument_symbol
  : type_compile_exception
{
    int_invalid_argument_symbol(const symbol_source& src)
      : type_compile_exception("type_compile_int_invalid_argument_symbol", to_error_parameter(src))
    {}
};
struct int_invalid_argument_literal
  : type_compile_exception
{
    int_invalid_argument_literal(const symbol_source& src)
      : type_compile_exception("type_compile_int_invalid_argument_literal", to_error_parameter(src))
    {}
};
struct int_out_of_range_bit_width
  : type_compile_exception
{
    static const char* const str;
    int_out_of_range_bit_width(const symbol_source& src)
      : type_compile_exception("type_compile_int_out_of_range_bit_width", to_error_parameter(src))
    {}
};

}

#endif

