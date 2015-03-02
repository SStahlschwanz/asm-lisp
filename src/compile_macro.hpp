#ifndef COMPILE_MACRO_HPP_
#define COMPILE_MACRO_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"

namespace llvm
{
struct Value;
struct Function;
}

struct value_info
{
    const list_symbol& definition;
    const ref_symbol& name;
    const type_symbol& type;
    llvm::Value* llvm_value;
};


macro_symbol compile_macro(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);

std::pair<std::unique_ptr<llvm::Function>, std::unordered_map<identifier_id_t, value_info>> compile_signature(const symbol& params_node, const symbol& return_type_node, compilation_context& context);

#endif

