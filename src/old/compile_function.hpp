#ifndef COMPILE_FUNCTION_HPP_
#define COMPILE_FUNCTION_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"
#include "compile_type.hpp"

#include <utility>
#include <memory>
#include <unordered_map>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>

namespace instruction
{

struct add
{
    const node& statement;
    type_info type;
    llvm::Value* llvm_value;
};
struct sub
{
    const node& statement;
    type_info type;
    llvm::Value* llvm_value;
};
struct mul
{
    const node& statement;
    type_info type;
    llvm::Value* llvm_value;
};
struct sdiv
{
    const node& statement;
    type_info type;
    llvm::Value* llvm_value;
};

struct cmp
{
    const node& statement;
    std::size_t cmp_kind;
    type_info type;
    llvm::Value* llvm_value;
};

struct typed_alloc
{
    const node& statement;
    type_info type;
    llvm::Value* llvm_value;
};
struct store
{
    const node& statement;
    type_info type;
};
struct load
{
    const node& statement;
    type_info type;
    llvm::Value* llvm_value;
};

struct branch
{
    const node& statement;
};
struct cond_branch
{
    const node& statement;
};
struct phi
{
    const node& statement;
    type_info type;
    llvm::Value* llvm_value;
};
struct return_inst
{
    const node& statement;
    type_info type;
};
struct call
{
    const node& statement;
    type_info type;
    llvm::Value* llvm_value;
};

struct is_id
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct is_lit
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct is_ref
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct is_list
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct is_macro
{
    const node& statement;
    llvm::Value* llvm_value;
};

struct lit_create
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct lit_size
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct lit_push
{
    const node& statement;
};
struct lit_pop
{
    const node& statement;
};
struct lit_get
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct lit_set
{
    const node& statement;
};

struct list_create
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct list_size
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct list_push
{
    const node& statement;
};
struct list_pop
{
    const node& statement;
};
struct list_get
{
    const node& statement;
    llvm::Value* llvm_value;
};
struct list_set
{
    const node& statement;
};

}

variadic_make_variant
<
    instruction::add,
    instruction::sub,
    instruction::mul,
    instruction::div,
    instruction::typed_alloc,
    instruction::store,
    instruction::load,
    instruction::cond_branch,
    instruction::branch,
    instruction::phi,
    instruction::cmp,
    instruction::return_inst,
    instruction::call,

    instruction::is_id,
    instruction::is_lit,
    instruction::is_ref,
    instruction::is_list,
    instruction::is_macro,

    instruction::lit_create,
    instruction::lit_size,
    instruction::lit_push,
    instruction::lit_pop,
    instruction::lit_get,
    instruction::lit_set,

    instruction::list_create,
    instruction::list_size,
    instruction::list_push,
    instruction::list_pop,
    instruction::list_get,
    instruction::list_set
>::type statement;

struct statement_context
{
    llvm::IRBuilder<>& builder;
    std::function<statement& (const ref_symbol&)> lookup_variable;
    const macro_execution_environment& macro_environment;
};
struct block_info
{
    const ref_symbol& block_name;
    std::unordered_map<identifier_id_t, named_value_info> variable_table;
    std::vector<instruction> instructions;
    llvm::BasicBlock* llvm_block;
};
struct function_info
{
    llvm::Function* llvm_function;
    special_calls_info special_calls;
};


instruction_info parse_instruction(const symbol& node, llvm::LLVMContext& llvm_context);

boost::optional<named_value_info> compile_statement(const symbol& node, statement_context& st_context);

block_info compile_block(const symbol& block_node, llvm::BasicBlock& llvm_block, const std::function<named_value_info& (identifier_id_t)>& lookup_global_variable, special_calls_info& special_calls, compilation_context& context);

std::pair<std::unique_ptr<llvm::Function>, std::unordered_map<identifier_id_t, named_value_info>> compile_signature(const symbol& params_node, const symbol& return_type_node, compilation_context& context);

std::pair<std::unique_ptr<llvm::Function>, function_info> compile_function(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);

macro_symbol compile_macro(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);
proc_symbol compile_proc(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);

#endif

