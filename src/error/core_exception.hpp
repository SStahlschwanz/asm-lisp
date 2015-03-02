#ifndef CORE_EXCEPTION_HPP_
#define CORE_EXCEPTION_HPP_

#include "compile_exception.hpp"

namespace core_exception
{

struct core_exception
  : compile_exception
{
    using compile_exception::compile_exception;
};


struct identifier_not_defined
  : core_exception
{
    identifier_not_defined(const symbol_source& src)
      : core_exception{"core_exception_identifier_not_defined", to_error_parameter(src)}
    {}
};

}

#endif

