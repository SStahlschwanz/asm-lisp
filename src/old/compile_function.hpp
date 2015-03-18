#ifndef COMPILE_FUNCTION_HPP_
#define COMPILE_FUNCTION_HPP_

#include "node.hpp"
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

struct add_data
{
    type_info type;
    llvm::Value& llvm_value;
};
struct sub_data
{
    type_info type;
    llvm::Value& llvm_value;
};
struct mul_data
{
    type_info type;
    llvm::Value& llvm_value;
};
struct sdiv_data
{
    type_info type;
    llvm::Value& llvm_value;
};

struct cmp_data
{
    const id_symbol& cmp_kind;
    type_info type;
    llvm::Value& llvm_value;
};

struct typed_alloc_data
{
    type_info type;
    llvm::Value& llvm_value;
};
struct store_data
{
    type_info type;
};
struct load_data
{
    type_info type;
    llvm::Value& llvm_value;
};

struct branch_data
{
};
struct cond_branch_data
{
};
struct phi_data
{
    type_info type;
    llvm::Value& llvm_value;
};
struct return_inst_data
{
    type_info type;
};
struct call_data
{
    type_info type;
    llvm::Value& llvm_value;
};

struct is_id_data
{
    llvm::Value& llvm_value;
};
struct is_lit_data
{
    llvm::Value& llvm_value;
};
struct is_ref_data
{
    llvm::Value& llvm_value;
};
struct is_list_data
{
    llvm::Value& llvm_value;
};
struct is_macro_data
{
    llvm::Value& llvm_value;
};

struct lit_create_data
{
    llvm::Value& llvm_value;
};
struct lit_size_data
{
    llvm::Value& llvm_value;
};
struct lit_push_data
{
};
struct lit_pop_data
{
};
struct lit_get_data
{
    llvm::Value& llvm_value;
};
struct lit_set_data
{
};

struct list_create_data
{
    llvm::Value& llvm_value;
};
struct list_size_data
{
    llvm::Value& llvm_value;
};
struct list_push_data
{
};
struct list_pop_data
{
};
struct list_get_data
{
    llvm::Value& llvm_value;
};
struct list_set_data
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
>::type instruction_data;

typedef std::tuple<const node&, instruction_data> statement;

struct statement_context
{
    llvm::IRBuilder<>& builder;

    std::function<statement& (const ref_symbol&)> lookup_variable;
    const macro_execution_environment& macro_environment;
};
struct block_info
{
    const ref_symbol& block_name;
    std::unordered_map<std::string, statement&> variable_table;
    std::vector<instruction> instructions;
    llvm::BasicBlock* llvm_block;
};
struct function_info
{
    llvm::Function* llvm_function;
    special_calls_info special_calls;
};

statement_data

instruction_info parse_instruction(const symbol& node, llvm::LLVMContext& llvm_context);

boost::optional<named_value_info> compile_statement(const symbol& node, statement_context& st_context);

block_info compile_block(const symbol& block_node, llvm::BasicBlock& llvm_block, const std::function<named_value_info& (identifier_id_t)>& lookup_global_variable, special_calls_info& special_calls, compilation_context& context);

std::pair<std::unique_ptr<llvm::Function>, std::unordered_map<identifier_id_t, named_value_info>> compile_signature(const symbol& params_node, const symbol& return_type_node, compilation_context& context);

std::pair<std::unique_ptr<llvm::Function>, function_info> compile_function(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);

macro_symbol compile_macro(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);
proc_symbol compile_proc(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context);

#endif

