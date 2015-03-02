#ifndef UNIQUE_EXCEPTION_HPP_
#define UNIQUE_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

namespace unique_exception
{

struct unique_exception
  : compile_exception
{
    using compile_exception::compile_exception;
};

struct invalid_argument_number
  : unique_exception
{
    invalid_argument_number()
      : unique_exception{"unique_invalid_argument_number"}
    {}
};

}

#endif

