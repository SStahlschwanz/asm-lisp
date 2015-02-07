#ifndef DEFINE_ERROR_HPP_
#define DEFINE_ERROR_HPP_

#include <stdexcept>

class define_error
  : public std::runtime_error
{
    using runtime_error::runtime_error;
};

#endif

