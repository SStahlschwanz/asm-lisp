#ifndef IMPORT_EXPORT_ERROR_HPP_
#define IMPORT_EXPORT_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace import_export_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"import_invalid_argument_number", ""},
    {"invalid_import_list", ""},
    {"invalid_from_token", ""},
    {"invalid_imported_module", ""},
    {"invalid_imported_identifier", ""},
    {"import_after_header", ""},
    {"export_after_header", ""},
    {"symbol_not_found", ""},
    {"module_not_found", ""}
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

