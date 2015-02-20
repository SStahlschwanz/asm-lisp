#ifndef IMPORT_ERROR_HPP_
#define IMPORT_ERROR_HPP_

#include "../symbol.hpp"

#include <exception>

enum class import_error
  : unsigned int
{
    INVALID_ARGUMENT_NUMBER,
    INVALID_MODULE_NAME,
    INVALID_IMPORT_LIST,
    INVALID_IMPORT_IDENTIFIER,
    SYMBOL_NOT_FOUND,
    IMPORT_EXPORT_NOT_AT_BEGIN,
    MISSING_FROM_IDENTIFIER,

    EXPORT_TOO_FEW_ARGUMENTS,
    INVALID_EXPORT_IDENTIFIER,
    EXPORT_UNDEFINED
};

constexpr const char* import_error_strings[] =
{
    "\"import\" takes exactly two arguments",
    "invalid module name: expected identifier",
    "invalid import identifier list: expected list",
    "invalid import symbol: expected identifier",
    "symbol was not found in imported module",
    "import or export statement not at beginning of file",
    "expected \"from\"",
    
    "\"export\" takes at least one argument",
    "invalid export symbol: expected identifier",
    "undefined exported symbol"
};

class import_exception
  : public std::exception
{
public:
    import_exception(symbol::source_type source, import_error error)
      : source(std::move(source)),
        error(error)
    {}
    const char* what() const noexcept override
    {
        return import_error_strings[(unsigned int)error];
    }

    symbol::source_type source;
    import_error error;
};

#endif

