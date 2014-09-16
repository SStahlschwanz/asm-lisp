#ifndef IMPORT_ERROR_HPP_
#define IMPORT_ERROR_HPP_

#include <stdexcept>

struct import_error
  : public std::runtime_error
{
    using runtime_error::runtime_error;
};

#endif

