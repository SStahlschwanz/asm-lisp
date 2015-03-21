#ifndef INSTRUCTION_TYPES_HPP_
#define INSTRUCTION_TYPES_HPP_

#include "compile_type.hpp"
#include "node.hpp"

#include <mblib/boost_variant.hpp>

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>


namespace instruction
{

struct add
{
    type_info type;
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct sub
{
    type_info type;
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct mul
{
    type_info type;
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct sdiv
{
    type_info type;
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};

struct cmp
{
    const id_node& cmp_kind;
    type_info type;
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};

struct typed_alloc
{
    type_info type;
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct store
{
    type_info type;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct load
{
    type_info type;
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};

struct branch
{
    const ref_node& block_name;

    llvm::BranchInst* value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct cond_branch
{
    const ref_node& true_block_name;
    const ref_node& false_block_name;

    llvm::Value& boolean;
    llvm::BranchInst* value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct phi
{
    struct incoming
    {
        const ref_node& variable_name;
        const ref_node& block_name;
    };

    type_info type;
    std::vector<incoming> incomings;

    llvm::PHINode& llvm_value;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct return_inst
{
    type_info type;

    static constexpr bool is_ct_only = false;
    static constexpr bool is_rt_only = false;
};
struct call
{
    type_info return_type;
    std::vector<type_info> arg_types;
    const proc_node& callee;

    llvm::CallInst& llvm_value;

    bool is_ct_only;
    bool is_rt_only;
};

struct is_id
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct is_lit
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct is_ref
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct is_list
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct is_macro
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};

struct lit_create
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct lit_size
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct lit_push
{

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct lit_pop
{

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct lit_get
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct lit_set
{

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};

struct list_create
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct list_size
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct list_push
{

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct list_pop
{

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct list_get
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct list_set
{
    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};

struct ref_create
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct ref_get_identifier
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct ref_set_identifier
{
    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct ref_has_refered
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct ref_get_refered
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct ref_set_refered
{
    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};

struct to_node
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};
struct call_macro
{
    llvm::Value& llvm_value;

    static constexpr bool is_ct_only = true;
    static constexpr bool is_rt_only = false;
};

}

typedef variadic_make_variant
<
    instruction::add,
    instruction::sub,
    instruction::mul,
    instruction::sdiv,
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
    instruction::list_set,

    instruction::ref_create,
    instruction::ref_get_identifier,
    instruction::ref_set_identifier,
    instruction::ref_has_refered,
    instruction::ref_get_refered,
    instruction::ref_set_refered,

    instruction::to_node,
    instruction::call_macro
>::type instruction_data;

typedef std::pair<const node&, instruction_data> statement;

struct named_value_info
{
    llvm::Value& llvm_value;

    const ref_node& name_ref;
};

#endif

