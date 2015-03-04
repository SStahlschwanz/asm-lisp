#ifndef IMPORT_EXPORT_ERROR_HPP_
#define IMPORT_EXPORT_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace import_export_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"import_invalid_argument_number", "{0}: invalid number of arguments to \'import\': expected 3, got {1}"},
    {"invalid_import_list", "expected a list (of imports)"},
    {"invalid_from_token", "expected 'from'"},
    {"invalid_imported_module", "expected an identifier (module name)"},
    {"invalid_imported_identifier", "expected an identifier (name of symbol to import)"},
    {"import_after_header", "import statement after file header not allowed"},
    {"export_after_header", "export statement after file header not allowed"},
    {"symbol_not_found", "symbol not found"},
    {"module_not_found", "module not found"}
};

constexpr std::size_t id(conststr str)
{
    return id(str, dictionary);
}

template<std::size_t error_id, class Location, class... ParamTypes>
[[ noreturn ]] void fatal(Location location, ParamTypes&&... params)
{
    static_assert(error_id < size(dictionary), "invalid error id");
    static_assert(error_id != std::numeric_limits<std::size_t>::max(), "invalid error id");
    throw compile_exception{error_kind::IMPORT_EXPORT, error_id, location, std::forward<ParamTypes>(params)...};
}

}


#endif

