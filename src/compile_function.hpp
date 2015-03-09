#ifndef COMPILE_FUNCTION_HPP_
#define COMPILE_FUNCTION_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"
#include "compile_type.hpp"

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

struct instruction_info
{
    struct add
    {
        type_info type;
    };
    struct sub
    {
        type_info type;
    };
    struct mul
    {
        type_info type;
    };
    struct div
    {
        type_info type;
    };
    
    struct alloc
    {
        type_info type;
    };
    struct store
    {
        type_info type;
    };
    struct load
    {
        type_info type;
    };
    struct cond_branch
    {};
    struct branch
    {};
    struct phi
    {
        type_info type;
    };
    struct cmp
    {
        std::size_t cmp_kind;
        type_info type;
    };
    struct return_inst
    {
        type_info type;
    };
    struct call
    {
        type_info type;
    };

    struct list_create
    {
    };
    struct list_size
    {
    };
    struct list_push
    {
    };
    struct list_pop
    {
    };
    struct list_get
    {
    };
    struct list_set
    {
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
        phi,
        cmp,
        return_inst,
        call,

        list_create,
        list_size,
        list_push,
        list_pop,
        list_get,
        list_set
    > kind;
};

struct cond_branch_call
{
    llvm::BranchInst* value;
    const ref_symbol& true_block_name;
    const ref_symbol& false_block_name;
};
struct branch_call
{
    llvm::BranchInst* value;
    const ref_symbol& block_name;
};
struct phi_call
{
    llvm::PHINode* value;
    struct incoming
    {
        const ref_symbol& variable_name;
        const ref_symbol& block_name;
    };
    std::vector<incoming> incomings;
    const symbol& statement;
};
struct ct_only_call
{
    const symbol& instruction_node;
};
struct rt_only_call
{
    const symbol& instruction_node;
};

struct special_calls_info
{
    std::vector<branch_call> branches;
    std::vector<cond_branch_call> cond_branches;
    std::vector<phi_call> phis;
    std::vector<ct_only_call> ct_only_instructions;
    std::vector<rt_only_call> rt_only_instructions;
};

struct statement_context
{
    llvm::IRBuilder<>& builder;
    std::function<named_value_info& (const ref_symbol&)> lookup_variable;
    special_calls_info& special_calls;
    const macro_execution_environment& macro_environment;
};
struct block_info
{
    const ref_symbol& block_name;
    std::unordered_map<identifier_id_t, named_value_info> variable_table;
    llvm::BasicBlock* llvm_block;
};
struct function_info
{
    bool uses_proc_instructions;
    bool uses_macro_instructions;
    llvm::Function* llvm_function;
};


instruction_info parse_instruction(const symbol& node, llvm::LLVMContext& llvm_context);

boost::optional<named_value_info> compile_statement(const symbol& node, statement_context& st_context);

block_info compile_block(const symbol& block_node, llvm::BasicBlock& llvm_block, const std::function<named_value_info& (identifier_id_t)>& lookup_global_variable, special_calls_info& special_calls, compilation_context& context);

std::pair<std::unique_ptr<llvm::Function>, std::unordered_map<identifier_id_t, named_value_info>> compile_signature(const symbol& params_node, const symbol& return_type_node, compilation_context& context);

std::pair<std::unique_ptr<llvm::Function>, function_info> compile_function(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);

#endif

