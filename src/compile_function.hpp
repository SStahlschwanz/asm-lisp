#ifndef COMPILE_FUNCTION_HPP_
#define COMPILE_FUNCTION_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"

#include <utility>
#include <memory>
#include <unordered_map>

#include <boost/optional.hpp>

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>

struct named_value_info
{
    const list_symbol& definition;
    const ref_symbol& name;
    llvm::Value* llvm_value;
};

std::pair<std::unique_ptr<llvm::Function>, std::unordered_map<identifier_id_t, named_value_info>> compile_signature(const symbol& params_node, const symbol& return_type_node, compilation_context& context);


struct instruction_statement
{
    struct add
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        const type_symbol& type;
    };
    struct sub
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        const type_symbol& type;
    };
    struct mul
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        const type_symbol& type;
    };
    struct div
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        const type_symbol& type;
    };
    
    struct alloc
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        const type_symbol& type;
    };
    struct store
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        const type_symbol& type;
    };
    struct load
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        const type_symbol& type;
    };
    struct cond_branch
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;
    };
    struct branch
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;
    };
    struct cmp
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        std::size_t cmp_kind;
        const type_symbol& type;
    };
    struct return_inst
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

        const type_symbol& type;
    };
    struct call
    {
        static constexpr bool is_proc_only = false;
        static constexpr bool is_macro_only = false;

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
        branch,
        cmp,
        return_inst,
        call
    > instruction;
};

struct incomplete_cond_branch
{
    llvm::BranchInst* value;
    const ref_symbol& true_block_name;
    const ref_symbol& false_block_name;
};
struct incomplete_branch
{
    llvm::BranchInst* value;
    const ref_symbol& block_name;
};
struct incomplete_phi
{
    llvm::PHINode* value;
    struct incoming
    {
        const ref_symbol& block_name;
        const ref_symbol& variable_name;
    };
    std::vector<std::pair<const ref_symbol&, const ref_symbol&>> incoming;
};

typedef boost::variant
<
    incomplete_cond_branch,
    incomplete_branch,
    incomplete_phi
> incomplete_statement;

instruction_statement compile_instruction(const symbol& node);
template<class VariableLookupFunctor>
std::pair<boost::optional<named_value_info>, boost::optional<incomplete_statement>> compile_statement(const symbol& node, VariableLookupFunctor&& lookup_variable, llvm::IRBuilder<>& builder);

struct block_info
{
    const ref_symbol& block_name;
    std::unordered_map<identifier_id_t, named_value_info> variable_table;
    bool is_entry_block;
    llvm::BasicBlock* llvm_block;
    std::vector<incomplete_statement> incomplete_statements;
};
std::pair<block_info, std::unique_ptr<llvm::BasicBlock>> compile_block(const symbol& block_node, const std::unordered_map<identifier_id_t, named_value_info>& global_variable_table, compilation_context& context);

struct function_info
{
    bool uses_proc_instructions;
    bool uses_macro_instructions;
    llvm::Function* llvm_function;
};

std::pair<std::unique_ptr<llvm::Function>, function_info> compile_function(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);

#endif

