#ifndef CORE_MISC_ERROR_HPP_
#define CORE_MISC_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace core_misc_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"unique_invalid_argument_number", "invalid number of arguments to 'unique': expected 0"},
    {"identifier_not_defined", ""},
    {"external_invalid_argument_number", ""},
    {"external_not_a_signature_type", ""},
    {"external_invalid_name", ""},
    {"main_ct_only_proc", ""},
    {"main_invalid_signature", ""}
};

constexpr std::size_t id(conststr str)
{
    return index_of(str, dictionary);
}

template<std::size_t error_id, class Location, class... ParamTypes>
[[ noreturn ]] void fatal(Location location, ParamTypes&&... params)
{
    static_assert(error_id < size(dictionary), "invalid error id");
    static_assert(error_id != std::numeric_limits<std::size_t>::max(), "invalid error id");
    throw compile_exception{error_kind::CORE_MISC, error_id, location, std::forward<ParamTypes>(params)...};
}

}

#endif

