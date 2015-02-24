#ifndef TYPE_COMPILATION_EXCEPTION_HPP_
#define TYPE_COMPILATION_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

class type_compilation_exception
  : public compile_exception
{
public:
    template<class... Types>
    type_compilation_exception(Types&&... types)
    {}
};

class int_invalid_argument_number
  : public type_compilation_exception
{
public:
    using type_compilation_exception::type_compilation_exception;
};
class int_invalid_argument_symbol
  : public type_compilation_exception
{
public:
    using type_compilation_exception::type_compilation_exception;
};
class int_invalid_argument_literal
  : public type_compilation_exception
{
public:
    using type_compilation_exception::type_compilation_exception;
};
class int_out_of_range_bit_width
  : public type_compilation_exception
{
public:
    using type_compilation_exception::type_compilation_exception;
};
#endif

