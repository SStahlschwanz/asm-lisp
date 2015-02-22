#ifndef EVALUATION_EXCEPTION_HPP_
#define EVALUATION_EXCEPTION_HPP_

#include "../symbol.hpp"

#include <exception>

class evaluation_exception
  : public std::exception
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

#endif

