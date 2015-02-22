#ifndef IMPORT_EXPORTS_EXCEPTION_HPP_
#define IMPORT_EXPORTS_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

class import_export_exception
  : public compile_exception
{
public:
    template<class... Types>
    import_export_exception(Types&&... types)
    {}
};

class import_invalid_argument_number
  : public import_export_exception
{
public:
    using import_export_exception::import_export_exception;
};
class missing_import_list 
  : public import_export_exception
{
public:
    using import_export_exception::import_export_exception;
};
class missing_from_token
  : public import_export_exception
{
public:
    using import_export_exception::import_export_exception;
};
class invalid_imported_module
  : public import_export_exception
{
public:
    using import_export_exception::import_export_exception;
};
class invalid_imported_identifier
  : public import_export_exception
{
public:
    using import_export_exception::import_export_exception;
};
class import_after_header
  : public import_export_exception
{
public:
    using import_export_exception::import_export_exception;
};
class export_after_header
  : public import_export_exception
{
public:
    using import_export_exception::import_export_exception;
};
class symbol_not_found
  : public import_export_exception
{
public:
    using import_export_exception::import_export_exception;
};

#endif

