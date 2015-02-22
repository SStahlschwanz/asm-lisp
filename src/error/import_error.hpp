#ifndef IMPORT_ERROR_HPP_
#define IMPORT_ERROR_HPP_

#include "../symbol.hpp"

#include <exception>

class import_exception
  : public std::exception
{
public:
    template<class... Types>
    import_exception(Types&&... types)
    {}
};

class import_invalid_argument_number
  : public import_exception
{
public:
    using import_exception::import_exception;
};
class missing_import_list 
  : public import_exception
{
public:
    using import_exception::import_exception;
};
class missing_from_token
  : public import_exception
{
public:
    using import_exception::import_exception;
};
class invalid_imported_module
  : public import_exception
{
public:
    using import_exception::import_exception;
};
class invalid_imported_identifier
  : public import_exception
{
public:
    using import_exception::import_exception;
};
class import_after_header
  : public import_exception
{
public:
    using import_exception::import_exception;
};
class export_after_header
  : public import_exception
{
public:
    using import_exception::import_exception;
};

#endif

