#ifndef EVALUATION_EXCEPTION_HPP_
#define EVALUATION_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

class evaluation_exception
  : public compile_exception
{
public:
    template<class... Types>
    evaluation_exception(Types&&... types)
    {}
};

struct empty_top_level_statement
  : evaluation_exception
{
    using evaluation_exception::evaluation_exception;
};
struct invalid_command
  : evaluation_exception
{
    using evaluation_exception::evaluation_exception;
};
struct def_not_enough_arguments
  : evaluation_exception
{
    using evaluation_exception::evaluation_exception;
};
struct invalid_defined_name
  : evaluation_exception
{
    using evaluation_exception::evaluation_exception;
};
struct duplicate_definition
  : evaluation_exception
{
    using evaluation_exception::evaluation_exception;
};

#endif

