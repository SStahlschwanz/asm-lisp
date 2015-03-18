#ifndef MACRO_EXECUTION_ERROR_HPP_ 
#define MACRO_EXECUTION_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace macro_execution_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"generic_error", ""}
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
    throw compile_exception{error_kind::MACRO_EXECUTION, error_id, location, std::forward<ParamTypes>(params)...};
}

}

#endif

