#ifndef IMPORT_EXPORTS_EXCEPTION_HPP_
#define IMPORT_EXPORTS_EXCEPTION_HPP_

#include "../symbol.hpp"
#include "compile_exception.hpp"

namespace import_export_exception
{

struct import_export_exception
  : compile_exception
{
    using compile_exception::compile_exception;
};

struct import_invalid_argument_number
  : import_export_exception
{
    import_invalid_argument_number(const symbol_source& source)
      : import_export_exception{"import_export_import_invalid_argument_number", to_error_parameter(source)}
    {}
    using import_export_exception::import_export_exception;
};
struct invalid_import_list 
  : import_export_exception
{
    invalid_import_list(const symbol_source& source)
      : import_export_exception{"import_export_invalid_import_list", to_error_parameter(source)}
    {}
    using import_export_exception::import_export_exception;
};
struct invalid_from_token
  : import_export_exception
{
    invalid_from_token(const symbol_source& source)
      : import_export_exception{"import_export_invalid_from_token", to_error_parameter(source)}
    {}
};
struct invalid_imported_module
  : import_export_exception
{
    invalid_imported_module(const symbol_source& source)
      : import_export_exception{"import_export_invalid_imported_module", to_error_parameter(source)}
    {}
};
struct invalid_imported_identifier
  : import_export_exception
{
    invalid_imported_identifier(const symbol_source& source)
      : import_export_exception{"import_export_invalid_imported_identifier", to_error_parameter(source)}
    {}
};
struct import_after_header
  : import_export_exception
{
    import_after_header(const symbol_source& source)
      : import_export_exception{"import_export_import_after_header", to_error_parameter(source)}
    {}
};
struct export_after_header
  : import_export_exception
{
    export_after_header(const symbol_source& source)
      : import_export_exception{"import_export_export_after_header", to_error_parameter(source)}
    {}
};
struct symbol_not_found
  : import_export_exception
{
    symbol_not_found(const symbol_source& source)
      : import_export_exception{"import_export_symbol_not_found", to_error_parameter(source)}
    {}
};

}

#endif

