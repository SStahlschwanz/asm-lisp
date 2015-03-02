#ifndef COMPILE_MACRO_HPP_
#define COMPILE_MACRO_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"

#include <utility>
#include <memory>
#include <unordered_map>

macro_symbol compile_macro(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);

// in header because of unit test:

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

std::pair<std::unique_ptr<llvm::Function>, std::unordered_map<identifier_id_t, value_info>> compile_signature(const symbol& params_node, const symbol& return_type_node, compilation_context& context);


struct instruction_statement
{
    struct add
    {
        const type_symbol& type;
    };
    struct sub
    {
        const type_symbol& type;
    };
    struct mul
    {
        const type_symbol& type;
    };
    struct div
    {
        const type_symbol& type;
    };
    
    struct alloc
    {
        const type_symbol& type;
    };
    struct store
    {
        const type_symbol& type;
    };
    struct load
    {
        const type_symbol& type;
    };
    struct cond_branch
    {};
    struct cmp
    {
        std::size_t cmp_kind;
        const type_symbol& type;
    };
    struct return_inst
    {
        const type_symbol& type;
    };
    struct call
    {
        const type_symbol& type;
    };
    
    const symbol& statement;
    boost::variant
    <
        add,
        sub,
        mul,
        div,
        alloc,
        store,
        load,
        cond_branch,
        cmp,
        return_inst,
        call
    > instruction;
};

instruction_statement compile_instruction(const symbol& node);

#endif

