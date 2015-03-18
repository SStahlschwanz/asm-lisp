#ifndef COMPILE_INSTRUCTION_HPP_
#define COMPILE_INSTRUCTION_HPP_

#include <iostream>
using std::cout;
using std::endl;

#include "compile_type.hpp"
#include "error/compile_instruction_error.hpp"
#include "node.hpp"
#include "core_unique_ids.hpp"
#include "core_utils.hpp"

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>

#include <boost/optional.hpp>

namespace instruction
{

struct add
{
    type_info type;
    llvm::Value& llvm_value;
};
struct sub
{
    type_info type;
    llvm::Value& llvm_value;
};
struct mul
{
    type_info type;
    llvm::Value& llvm_value;
};
struct sdiv
{
    type_info type;
    llvm::Value& llvm_value;
};

struct cmp
{
    const id_node& cmp_kind;
    type_info type;
    llvm::Value& llvm_value;
};

struct typed_alloc
{
    type_info type;
    llvm::Value& llvm_value;
};
struct store
{
    type_info type;
};
struct load
{
    type_info type;
    llvm::Value& llvm_value;
};

struct branch
{
    const ref_node& block_name;

    llvm::BranchInst* value;
};
struct cond_branch
{
    const ref_node& true_block_name;
    const ref_node& false_block_name;

    llvm::Value& boolean;
    llvm::BranchInst* value;
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
};
struct return_inst
{
    type_info type;
};
struct call
{
    type_info type;
    llvm::Value& llvm_value;
};

struct is_id
{
    llvm::Value& llvm_value;
};
struct is_lit
{
    llvm::Value& llvm_value;
};
struct is_ref
{
    llvm::Value& llvm_value;
};
struct is_list
{
    llvm::Value& llvm_value;
};
struct is_macro
{
    llvm::Value& llvm_value;
};

struct lit_create
{
    llvm::Value& llvm_value;
};
struct lit_size
{
    llvm::Value& llvm_value;
};
struct lit_push
{
};
struct lit_pop
{
};
struct lit_get
{
    llvm::Value& llvm_value;
};
struct lit_set
{
};

struct list_create
{
    llvm::Value& llvm_value;
};
struct list_size
{
    llvm::Value& llvm_value;
};
struct list_push
{
};
struct list_pop
{
};
struct list_get
{
    llvm::Value& llvm_value;
};
struct list_set
{
    const node& statement;
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
    instruction::list_set
>::type instruction_data;

typedef std::pair<const node&, instruction_data> statement;

struct named_value_info
{
    llvm::Value& llvm_value;

    const ref_node& name_ref;
};

template<class DefineVariableFunctor, class LookupVariable, class AddStatementFunctor>
struct statement_context
{
    llvm::IRBuilder<>& builder;

    DefineVariableFunctor& define_variable;
    LookupVariable& lookup_variable;
    AddStatementFunctor& add_statement;

    //const macro_execution_environment& macro_environment;
};

template<class DefineVariableFunctor, class LookupVariable, class AddStatementFunctor>
statement_context<DefineVariableFunctor, LookupVariable, AddStatementFunctor> make_statement_context
(
    llvm::IRBuilder<>& builder,
    DefineVariableFunctor& define_variable,
    LookupVariable& lookup_variable,
    AddStatementFunctor& add_statement
)
{
    return {builder, define_variable, lookup_variable, add_statement};
}


template<class InstructionTypeRange, class ArgumentsRange, class StatementContext>
void compile_statement_impl
(
    const node& statement_node,
    const node& instruction_type_node,
    const ref_node* name_ref,
    InstructionTypeRange instruction_type_range,
    ArgumentsRange arguments_range,
    StatementContext st_context
)
{
    using namespace compile_instruction_error;
    using llvm::Value;
    using llvm::Type;
    using llvm::IRBuilder;
    using llvm::PointerType;
    using llvm::IntegerType;
    using llvm::isa;
    using llvm::ConstantInt;
    using llvm::LLVMContext;
    using llvm::PHINode;
    using boost::optional;
    using boost::none;
    using std::size_t;
    using std::move;
    using std::vector;
    using std::string;
    using std::stoul;
    using std::invalid_argument;
    using std::pair;
    using std::tuple;
    using std::out_of_range;


    if(instruction_type_range.empty())
        fatal<id("empty_instruction_type")>(instruction_type_node.source());

    const char* constructor_name;
    
    const id_node& instruction_type_constructor = instruction_type_range.front().template cast_else<id_node>([&]
    {
        fatal<id("invalid_instruction_type_constructor")>(instruction_type_range.front().source());
    });
    instruction_type_range.pop_front();


    IRBuilder<>& builder = st_context.builder;
    auto&& add_statement = st_context.add_statement;
    auto&& lookup_variable = st_context.lookup_variable;
    auto&& define_variable = st_context.define_variable;

    LLVMContext& llvm = builder.getContext();
    Type& pointer_type = *PointerType::getUnqual(IntegerType::get(builder.getContext(), 8));
    Type& symbol_type = *IntegerType::get(builder.getContext(), 64);
    Type& int64_type = *IntegerType::get(builder.getContext(), 64);
    Type& int1_type = *IntegerType::get(builder.getContext(), 1);
    Type& int8_type = *IntegerType::get(builder.getContext(), 8);


    auto add_instruction = [&](instruction_data data)
    {
        add_statement(statement_node, move(data));
    };

    auto result = [&](Value& val)
    {
        if(name_ref != nullptr)
            define_variable(*name_ref, named_value_info{val, *name_ref});

    };
    auto no_result = [&]()
    {
        if(name_ref != nullptr)
            fatal<id("let_for_instruction_without_result")>(name_ref->source());
    };

    auto check_constructor_arity = [&](size_t expected_arity)
    {
        if(length(instruction_type_range) != expected_arity)
            fatal<id("invalid_instruction_constructor_arity")>(instruction_type_constructor.source());
    };
    auto check_instruction_arity = [&](size_t expected_arity)
    {
        if(length(arguments_range) != expected_arity)
            fatal<id("invalid_instruction_arity")>(instruction_type_constructor.source());
    };

    auto get_constr_arg = [&]() -> const node&
    {
        const node& result = instruction_type_range.front();
        instruction_type_range.pop_front();
        return result;
    };
    auto get_arg = [&]() -> const node&
    {
        assert(!arguments_range.empty());
        const node& arg_node = arguments_range.front();
        arguments_range.pop_front();
        return arg_node;
    };
    auto get_typed_arg = [&](Type& expected_type) -> Value&
    {
        const node& arg_node = get_arg();
        
        return arg_node.template visit<Value&>(
        [&](const ref_node& variable_name) -> Value&
        {
            named_value_info info = lookup_variable(variable_name);
            Type* got_type = info.llvm_value.getType();
            if(&expected_type != got_type)
                fatal<id("variable_type_mismatch")>(variable_name.source(), expected_type, *got_type);
            return info.llvm_value;
        },
        [&](const lit_node& lit) -> Value&
        {
            if(!isa<IntegerType>(expected_type))
                fatal<id("invalid_literal_for_type")>(arg_node.source());
            
            long number;
            try
            {
                size_t index_after;
                string as_string{lit.begin(), lit.end()};
                number = stol(as_string, &index_after);
                if(index_after != as_string.size())
                    throw invalid_argument{""};
            }
            catch(const invalid_argument& exc)
            {
                fatal<id("invalid_integer_constant")>(lit.source());
            }
            catch(const out_of_range& exc)
            {
                fatal<id("out_of_range_integer_constant")>(lit.source());
            }
            
            if(!ConstantInt::isValueValidForType(&expected_type, number))
                fatal<id("out_of_range_integer_constant")>(lit.source());
            return *ConstantInt::getSigned(&expected_type, number);
        },
        [&](const node& n) -> Value&
        {
            fatal<id("invalid_value")>(n.source());
            return *static_cast<Value*>(nullptr);
        });
    };


    switch(instruction_type_constructor.id())
    {
        using namespace instruction;
        using namespace unique_ids;
        
        case ADD:
        {
            constructor_name = "add";
            check_constructor_arity(1);           
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(2);
            Value& arg1 = get_typed_arg(type.llvm_type);           
            Value& arg2 = get_typed_arg(type.llvm_type);
            Value& val = *builder.CreateAdd(&arg1, &arg2);

            result(val);
            add_instruction(add{move(type), val});
            break;
        }
        case SUB:
        {
            constructor_name = "sub";
            check_constructor_arity(1);           
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(2);
            Value& arg1 = get_typed_arg(type.llvm_type);           
            Value& arg2 = get_typed_arg(type.llvm_type);
            Value& val = *builder.CreateSub(&arg1, &arg2);

            result(val);
            add_instruction(sub{move(type), val});
            break;
        }
        case MUL:
        {
            constructor_name = "mul";
            check_constructor_arity(1);           
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(2);
            Value& arg1 = get_typed_arg(type.llvm_type);           
            Value& arg2 = get_typed_arg(type.llvm_type);
            Value& val = *builder.CreateMul(&arg1, &arg2);

            result(val);
            add_instruction(mul{move(type), val});
            break;
        }
        case SDIV:
        {
            constructor_name = "sdiv";
            check_constructor_arity(1);           
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(2);
            Value& arg1 = get_typed_arg(type.llvm_type);           
            Value& arg2 = get_typed_arg(type.llvm_type);
            Value& val = *builder.CreateSDiv(&arg1, &arg2);

            result(val);
            add_instruction(sdiv{move(type), val});
            break;
        }

        case ALLOC:
        {
            constructor_name = "alloc";
            check_constructor_arity(1);           
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(0);

            Value& typed_pointer = *builder.CreateAlloca(&type.llvm_type);
            Value& val = *builder.CreatePointerCast(&typed_pointer, &pointer_type);
            
            result(val);
            add_instruction(typed_alloc{move(type), val});
            break;
        }
        case STORE:
        {
            constructor_name = "store";
            check_constructor_arity(1);
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(2);
            Value& stored = get_typed_arg(type.llvm_type);
            Value& ptr = get_typed_arg(pointer_type);

            Type& typed_pointer_type = *PointerType::getUnqual(&type.llvm_type);
            Value& typed_pointer = *builder.CreatePointerCast(&ptr, &typed_pointer_type);
            builder.CreateStore(&stored, &typed_pointer);
            
            no_result();
            add_instruction(store{move(type)});
            break;
        }
        case LOAD:
        {
            constructor_name = "load";
            check_constructor_arity(1);
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(1);
            Value& ptr = get_typed_arg(pointer_type);

            Type& typed_pointer_type = *PointerType::getUnqual(&type.llvm_type);
            Value& typed_pointer = *builder.CreatePointerCast(&ptr, &typed_pointer_type);
            Value& val = *builder.CreateLoad(&typed_pointer);

            result(val);
            add_instruction(load{type, val});
            break;
        }

        case CMP:
        {
            constructor_name = "cmp";
            check_constructor_arity(2);
            const node& first_constr_arg = get_constr_arg();
            const id_node& cmp_kind = first_constr_arg.cast_else<id_node>([&]
            {
                fatal<id("invalid_comparison_kind_node")>(first_constr_arg.source());
            });
            
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(2);
            Value& arg1 = get_typed_arg(type.llvm_type);
            Value& arg2 = get_typed_arg(type.llvm_type);

            Value* val;
            switch(cmp_kind.id())
            {
            case unique_ids::EQ:
                val = builder.CreateICmpEQ(&arg1, &arg2);
                break;
            case unique_ids::NE:
                val = builder.CreateICmpNE(&arg1, &arg2);
                break;
            case unique_ids::LT:
                val = builder.CreateICmpSLT(&arg1, &arg2);
                break;
            case unique_ids::LE:
                val = builder.CreateICmpSLE(&arg1, &arg2);
                break;
            case unique_ids::GT:
                val = builder.CreateICmpSGT(&arg1, &arg2);
                break;
            case unique_ids::GE:
                val = builder.CreateICmpSGE(&arg1, &arg2);
                break;
            default:
                fatal<id("invalid_comparison_kind_id")>(cmp_kind.source());
                break;
            }

            result(*val);
            add_statement(statement_node, cmp{cmp_kind, move(type), *val});
            break;
        }
        case RETURN:
        {
            constructor_name = "return";
            check_constructor_arity(1);
            type_info type = compile_type(get_constr_arg(), llvm);

            check_instruction_arity(1);
            Value& arg = get_typed_arg(type.llvm_type);
            builder.CreateRet(&arg);
            //Value& val = *builder.CreateRet(&arg);

            no_result();
            add_statement(statement_node, return_inst{move(type)});
            break;
        }
        case COND_BRANCH:
        {
            constructor_name = "cond_branch";
            check_constructor_arity(0);

            check_instruction_arity(3);
            Value& boolean = get_typed_arg(int1_type);
            const node& first_node = get_arg();
            const ref_node& true_block = first_node.cast_else<ref_node>([&]
            {
                fatal<id("invalid_block_name")>(first_node.source());
            });
            const node& second_node = get_arg();
            const ref_node& false_block = second_node.cast_else<ref_node>([&]
            {
                fatal<id("invalid_block_name")>(second_node.source());
            });
            
            no_result();
            add_instruction(cond_branch{true_block, false_block, boolean, nullptr});
            break;
        }
        case BRANCH:
        {
            constructor_name = "branch";
            check_constructor_arity(0);

            check_instruction_arity(1);
            const node& arg = get_arg();
            const ref_node& block = arg.cast_else<ref_node>([&]
            {
                fatal<id("invalid_block_name")>(arg.source());
            });

            no_result();
            add_instruction(branch{block, nullptr});
            break;
        }
        case PHI:
        {
            constructor_name = "phi";
            check_constructor_arity(1);
            type_info type = compile_type(get_constr_arg(), llvm);

            if(arguments_range.empty())
                fatal<id("phi_empty_incomings")>(instruction_type_node.source());

            auto incomings = save<vector<phi::incoming>>(mapped(arguments_range,
            [&](const node& n) -> phi::incoming
            {
                const list_node& inc = n.cast_else<list_node>([&]
                {
                    fatal<id("phi_invalid_incoming_node")>(n.source());
                });

                if(inc.size() != 2)
                    fatal<id("phi_invalid_incoming_node_size")>(inc.source());
                
                const ref_node& variable_name = inc[0].cast_else<ref_node>([&]
                {
                    fatal<id("invalid_variable_name")>(inc[0].source());
                });
                const ref_node& block_name = inc[1].cast_else<ref_node>([&]
                {
                    fatal<id("invalid_block_name")>(inc[1].source());
                });
                
                return phi::incoming{variable_name, block_name};
            }));

            PHINode& val = *builder.CreatePHI(&type.llvm_type, incomings.size());

            result(val);
            add_instruction(phi{move(type), move(incomings), val});
            break;
        }
        case IS_ID:
        {
        }
        case IS_LIT:
        {
        }
        case IS_REF:
        {
        }
        case IS_LIST:
        {
        }
        case IS_MACRO:
        {
        }

        case LIT_CREATE:
        {
        }
        case LIT_SIZE:
        {
        }
        case LIT_PUSH:
        {
        }
        case LIT_POP:
        {
        }
        case LIT_GET:
        {
        }
        case LIT_SET:
        {
        }

        case LIST_CREATE:
        {
        }
        case LIST_SIZE:
        {
        }
        case LIST_PUSH:
        {
        }
        case LIST_POP:
        {
        }
        case LIST_GET:
        {
        }
        case LIST_SET:
        {
        }
        default:
            fatal<id("unknown_instruction_constructor")>(instruction_type_constructor.source());
    }
          
}


template<class StatementContext>
void compile_statement(const node& statement_node, StatementContext& st_context)
{
    using namespace compile_instruction_error;

    auto statement_range = rangeify(statement_node.cast_else<list_node>([&]
    {
        fatal<id("invalid_statement")>(statement_node.source());
    }));


    if(statement_range.empty())
        fatal<id("empty_statement")>(statement_node.source());

    const ref_node* name_node = nullptr;
    const node& resolved_first_node = resolve_refs(statement_range.front());
    if(resolved_first_node.is<id_node>())
    {
        const id_node& first_node = resolved_first_node.cast<id_node>();
        if(first_node.id() == unique_ids::LET)
        {
            statement_range.pop_front();
            if(statement_range.empty())
                fatal<id("let_invalid_argument_number")>(statement_node.source());

            name_node = &statement_range.front().template cast_else<ref_node>([&]
            {
                fatal<id("invalid_variable_name")>(statement_range.front().source());
            });
            statement_range.pop_front();
        }
    }

    if(statement_range.empty())
        fatal<id("empty_instruction")>(statement_node.source());

    const node& instruction_type_node = statement_range.front();
    statement_range.pop_front();
    auto& arguments_range = statement_range;


    instruction_type_node.visit(
    [&](const list_node& list)
    {
        auto instruction_type_range = rangeify(list);
        compile_statement_impl
        (
            statement_node,
            instruction_type_node,
            name_node,
            instruction_type_range,
            arguments_range,
            st_context
        );
    },
    [&](const node& any_node)
    {
        auto instruction_type_range = static_range(any_node);
        compile_statement_impl
        (
            statement_node,
            instruction_type_node,
            name_node,
            instruction_type_range,
            arguments_range,
            st_context
        );
    });
}

#endif

