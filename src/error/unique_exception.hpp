#ifndef UNIQUE_EXCEPTION_HPP_
#define UNIQUE_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

class unique_exception
  : public compile_exception
{
public:
    template<class... Types>
    unique_exception(Types&&... types)
    {}
};

struct parameter_to_unique
  : unique_exception
{
    using unique_exception::unique_exception;
};

#endif

