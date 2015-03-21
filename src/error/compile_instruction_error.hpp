#ifndef COMPILE_INSTRUCTION_ERROR_HPP_
#define COMPILE_INSTRUCTION_ERROR_HPP_

#include "error_utils.hpp"
#include "compile_exception.hpp"

namespace compile_instruction_error
{

constexpr std::pair<conststr, conststr> dictionary[] =
{
    {"empty_statement", ""},
    {"empty_instruction", ""},
    {"empty_instruction_type", ""},
    {"invalid_instruction_type_constructor", ""},
    {"invalid_instruction_constructor_arity", ""},
    {"invalid_instruction_arity", ""},
    {"invalid_comparison_kind_node", ""},
    {"invalid_comparison_kind_id", ""},
    {"invalid_block_name", ""},
    {"phi_empty_incomings", ""},
    {"phi_invalid_incoming_node", ""},
    {"phi_invalid_incoming_node_size", ""},
    {"invalid_variable_name", ""},
    {"variable_type_mismatch", ""},
    {"invalid_literal_for_type", ""},
    {"invalid_integer_constant", ""},
    {"out_of_range_integer_constant", ""},
    {"invalid_value", ""},
    {"invalid_statement", ""},
    {"empty_statement", ""},
    {"let_invalid_argument_number", ""},
    {"let_for_instruction_without_result", ""},
    {"unknown_instruction_constructor", ""},
    {"call_invalid_argument_type_list", ""},
    {"call_invalid_callee", ""},
    {"call_signature_mismatch", ""},
    {"call_macro_invalid_macro", ""}
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
    throw compile_exception{error_kind::COMPILE_INSTRUCTION, error_id, location, std::forward<ParamTypes>(params)...};
}

}

#endif

