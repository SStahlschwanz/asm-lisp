#include "compile_function.hpp"
#include "error/compile_function_error.hpp"
#include "error/core_misc_error.hpp"
#include "core_unique_ids.hpp"
#include "boost_variant_utils.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Verifier.h>

#include <boost/optional.hpp>

#include <algorithm>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <string>

using std::distance;
using std::forward;
using std::pair;
using std::tie;
using std::ignore;
using std::vector;
using std::unordered_map;
using std::find;
using std::unique_ptr;
using std::move;
using std::begin;
using std::end;
using std::string;
using std::stol;
using std::out_of_range;
using std::invalid_argument;

using llvm::Value;
using llvm::Function;
using llvm::FunctionType;
using llvm::LLVMContext;
using llvm::Type;
using llvm::BasicBlock;
using llvm::isa;
using llvm::IntegerType;
using llvm::IRBuilder;
using llvm::ConstantInt;
using llvm::PointerType;
using llvm::BranchInst;
using llvm::PHINode;
using llvm::verifyFunction;

using boost::get;
using boost::apply_visitor;
using boost::static_visitor;
using boost::optional;
using boost::none;
using boost::blank;

using namespace compile_function_error;

const symbol& resolve_refs(const symbol& s)
{
    if(s.is<ref_symbol>())
    {
        const ref_symbol& r = s.cast<ref_symbol>();
        if(r.refered() == 0)
            core_misc_error::fatal<core_misc_error::id("identifier_not_defined")>(s.source());
        else
            return resolve_refs(*r.refered());
    }
    else
        return s;
}


pair<unique_ptr<Function>, unordered_map<identifier_id_t, named_value_info>> compile_signature(const symbol& params_node, const symbol& return_type_node, compilation_context& context)
{
    const list_symbol& params_list = params_node.cast_else<list_symbol>([&]()
    {
        fatal<id("invalid_param_list")>(params_node.source());
    });
    
    const symbol& resolved_return_symbol = resolve_refs(return_type_node);
    const type_symbol& return_type = resolved_return_symbol.cast_else<type_symbol>([&]()
    {
        fatal<id("invalid_return_type")>(resolved_return_symbol.source());
    });
    
    vector<named_value_info> arg_infos;
    arg_infos.reserve(params_list.size());
    vector<Type*> arg_types;
    arg_types.reserve(params_list.size());
    for(const symbol& s : params_list)
    {
        const list_symbol& param_declaration = s.cast_else<list_symbol>([&]()
        {
            fatal<id("invalid_parameter_declaration")>(s.source());
        });
        if(param_declaration.size() != 2)
            fatal<id("invalid_parameter_declaration_node_number")>(param_declaration.source());
        
        const ref_symbol& param_name = param_declaration[0].cast_else<ref_symbol>([&]()
        {
            fatal<id("invalid_parameter_name")>(param_declaration[0].source());
        });

        const symbol& resolved_param_type_symbol = resolve_refs(param_declaration[1]);
        const type_symbol& param_type = resolved_param_type_symbol.cast_else<type_symbol>([&]()
        {
            fatal<id("invalid_parameter_type")>(resolved_param_type_symbol.source());
        });
        arg_infos.push_back(named_value_info{param_declaration, param_name, nullptr}); // llvm_value is set later
        arg_types.push_back(param_type.llvm_type());
    }
    
    FunctionType* function_type = FunctionType::get(return_type.llvm_type(), arg_types, false);
    unique_ptr<Function> function{Function::Create(function_type, Function::InternalLinkage)};
    unordered_map<identifier_id_t, named_value_info> parameter_table;
    
    auto arg_it = function->arg_begin();
    auto info_it = arg_infos.begin();
    for( ; arg_it != function->arg_end(); ++arg_it, ++info_it)
    {
        assert(info_it != arg_infos.end());
        info_it->llvm_value = arg_it;
        identifier_id_t arg_identifier = info_it->name.identifier();
        arg_it->setName(context.to_string(arg_identifier));
        bool was_inserted;
        tie(ignore, was_inserted) = parameter_table.insert({arg_identifier, *info_it});
        if(!was_inserted)
            fatal<id("duplicate_parameter_name")>(info_it->name.source());
    }
    assert(info_it == arg_infos.end());

    return {move(function), move(parameter_table)};
}

template<class InstructionType>
instruction_statement compile_unary_typed_instruction(const char* name, const list_symbol& statement)
{
    if(statement.size() != 2)
        fatal<id("instruction_constructor_invalid_argument_number")>(statement.source(), name, 1, statement.size() - 1);

    const symbol& resolved_param = resolve_refs(statement[1]);
    const type_symbol& type = resolved_param.cast_else<type_symbol>([&]
    {
        fatal<id("invalid_instruction_type_parameter")>(resolved_param.source());
    });
    return instruction_statement{statement, InstructionType{type}};
}
template<class InstructionType>
instruction_statement compile_number_instruction(const char* name, const list_symbol& statement)
{
    instruction_statement result = compile_unary_typed_instruction<InstructionType>(name, statement);
    const type_symbol& type = get<InstructionType>(result.instruction).type;
    if(!isa<IntegerType>(type.llvm_type()))
        fatal<id("invalid_number_type")>(type.source());
    return result;
}
instruction_statement compile_cmp_instruction(const list_symbol& statement)
{
    if(statement.size() != 3)
        fatal<id("instruction_constructor_invalid_argument_number")>(statement.source(), "cmp", 2, statement.size() - 1);

    const symbol& resolved_first_arg = resolve_refs(statement[1]);
    const symbol& resolved_second_arg = resolve_refs(statement[2]);
    
    const id_symbol& first_arg = resolved_first_arg.cast_else<id_symbol>([&]
    {
        fatal<id("invalid_comparison_kind")>(resolved_first_arg.source());
    });
    size_t comparison_kind = first_arg.id();
    const size_t known_comparison_kinds[] =
    {
        unique_ids::EQ,
        unique_ids::NE,
        unique_ids::LT,
        unique_ids::LE,
        unique_ids::GT,
        unique_ids::GE
    };
    if(find(begin(known_comparison_kinds), end(known_comparison_kinds), comparison_kind) == end(known_comparison_kinds))
        fatal<id("unknown_comparison_kind")>(resolved_first_arg.source());

    const type_symbol& second_arg = resolved_second_arg.cast_else<type_symbol>([&]
    {
        fatal<id("invalid_instruction_type_parameter")>(resolved_second_arg.source());
    });

    return instruction_statement{statement, instruction_statement::cmp{comparison_kind, second_arg}};
}

instruction_statement compile_instruction(const symbol& node)
{
    if(node.is<list_symbol>())
    {
        const list_symbol& statement = node.cast<list_symbol>();
        if(statement.empty())
            fatal<id("empty_instruction")>(statement.source());
        const symbol& resolved_instruction_constructor = resolve_refs(statement[0]);
        const id_symbol& instruction_constructor = resolved_instruction_constructor.cast_else<id_symbol>([&]
        {
            fatal<id("invalid_instruction_constructor")>(statement[0].source());
        });
        switch(instruction_constructor.id())
        {
        case unique_ids::ADD:
            return compile_number_instruction<instruction_statement::add>("add", statement);
        case unique_ids::SUB:
            return compile_number_instruction<instruction_statement::sub>("sub", statement);
        case unique_ids::MUL:
            return compile_number_instruction<instruction_statement::mul>("mul", statement);
        case unique_ids::DIV:
            return compile_number_instruction<instruction_statement::div>("div", statement);
        case unique_ids::ALLOC:
            return compile_unary_typed_instruction<instruction_statement::alloc>("alloc", statement);
        case unique_ids::STORE:
            return compile_unary_typed_instruction<instruction_statement::store>("store", statement);
        case unique_ids::LOAD:
            return compile_unary_typed_instruction<instruction_statement::load>("load", statement);
        case unique_ids::RETURN:
            return compile_unary_typed_instruction<instruction_statement::return_inst>("return", statement);
        case unique_ids::COND_BRANCH:
            if(statement.size() != 1)
                fatal<id("instruction_constructor_invalid_argument_number")>(statement.source(), "cond_branch", 0, statement.size() - 1);
            return instruction_statement{statement, instruction_statement::cond_branch{}};
        case unique_ids::BRANCH:
            if(statement.size() != 1)
                fatal<id("instruction_constructor_invalid_argument_number")>(statement.source(), "branch", 0, statement.size() - 1);
            return instruction_statement{statement, instruction_statement::branch{}};
        case unique_ids::PHI:
            return compile_unary_typed_instruction<instruction_statement::phi>("phi", statement);
        case unique_ids::CMP:
            return compile_cmp_instruction(statement);
        case unique_ids::CALL:
            throw not_implemented{"instruction"};
        default:
            fatal<id("unknown_instruction_constructor")>(instruction_constructor.source());
        }
    }
    else
        throw not_implemented{"instruction is not a list"};
}

template<class VariableLookupFunctor>
pair<Value*, optional<incomplete_statement>> compile_instruction_call(list_symbol::const_iterator begin, list_symbol::const_iterator end, VariableLookupFunctor&& lookup_variable, IRBuilder<>& builder)
{
    assert(begin != end);
    instruction_statement instruction = compile_instruction(*begin);
    ++begin;

    size_t argument_number = distance(begin, end);
    auto check_arity = [&](const char* instruction_name, size_t expected_argument_number)
    {
        if(argument_number != expected_argument_number)
            fatal<id("invalid_instruction_call_argument_number")>(instruction.statement.source(), instruction_name, expected_argument_number, argument_number);
    };
    
    auto get_value = [&](const symbol& arg_node, Type* expected_type) -> Value*
    {
        if(arg_node.is<ref_symbol>())
        {
            const ref_symbol& name = arg_node.cast<ref_symbol>();
            optional<named_value_info> value = lookup_variable(name.identifier());
            if(!value)
                fatal<id("variable_undefined")>(name.source());
            if(expected_type != value->llvm_value->getType())
                fatal<id("variable_type_mismatch")>(name.source());
            return value->llvm_value;
        }
        else if(arg_node.is<lit_symbol>())
        {
            const lit_symbol& lit = arg_node.cast<lit_symbol>();
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
            
            if(!ConstantInt::isValueValidForType(expected_type, number))
                fatal<id("out_of_range_integer_constant")>(lit.source());
            return ConstantInt::getSigned(expected_type, number);
        }
        else
            fatal<id("invalid_value")>(arg_node.source());
    };
    
    typedef pair<Value*, optional<incomplete_statement>> return_type;
    return visit<return_type>(instruction.instruction,
    [&](const instruction_statement::add& inst) -> return_type
    {
        check_arity("add", 2);
        Value* arg1 = get_value(*begin, inst.type.llvm_type());
        Value* arg2 = get_value(*(begin + 1), inst.type.llvm_type());
        return {builder.CreateAdd(arg1, arg2), none};
    },
    [&](const instruction_statement::sub& inst) -> return_type
    {
        check_arity("sub", 2);
        Value* arg1 = get_value(*begin, inst.type.llvm_type());
        Value* arg2 = get_value(*(begin + 1), inst.type.llvm_type());
        return {builder.CreateSub(arg1, arg2), none};
    },
    [&](const instruction_statement::mul& inst) -> return_type
    {
        check_arity("mul", 2);
        Value* arg1 = get_value(*begin, inst.type.llvm_type());
        Value* arg2 = get_value(*(begin + 1), inst.type.llvm_type());
        return {builder.CreateMul(arg1, arg2), none};
    },
    [&](const instruction_statement::div& inst) -> return_type
    {
        check_arity("div", 2);
        Value* arg1 = get_value(*begin, inst.type.llvm_type());
        Value* arg2 = get_value(*(begin + 1), inst.type.llvm_type());
        return {builder.CreateSDiv(arg1, arg2), none};
    },
    [&](const instruction_statement::alloc& inst) -> return_type
    {
        check_arity("alloc", 0);
        return {builder.CreateAlloca(inst.type.llvm_type()), none};
    },
    [&](const instruction_statement::store& inst) -> return_type
    {
        check_arity("store", 2);
        Value* arg1 = get_value(*begin, inst.type.llvm_type());
        Type* ptr_type = PointerType::getUnqual(inst.type.llvm_type());
        Value* arg2 = get_value(*(begin + 1), ptr_type);
        return {builder.CreateStore(arg1, arg2), none};
    },
    [&](const instruction_statement::load& inst) -> return_type
    {
        check_arity("load", 1);
        Type* ptr_type = PointerType::getUnqual(inst.type.llvm_type());
        Value* arg = get_value(*begin, ptr_type);
        return {builder.CreateLoad(arg), none};
    },
    [&](const instruction_statement::cmp& inst) -> return_type
    {
        check_arity("cmp", 2);
        Value* arg1 = get_value(*begin, inst.type.llvm_type());
        Value* arg2 = get_value(*(begin + 1), inst.type.llvm_type());
        switch(inst.cmp_kind)
        {
        case unique_ids::EQ:
            return {builder.CreateICmpEQ(arg1, arg2), none};
        case unique_ids::NE:
            return {builder.CreateICmpNE(arg1, arg2), none};
        case unique_ids::LT:
            return {builder.CreateICmpSLT(arg1, arg2), none};
        case unique_ids::LE:
            return {builder.CreateICmpSLE(arg1, arg2), none};
        case unique_ids::GT:
            return {builder.CreateICmpSGT(arg1, arg2), none};
        case unique_ids::GE:
            return {builder.CreateICmpSGE(arg1, arg2), none};
        default:
            assert(false);
            return {nullptr, none};
        }
    }, 
    [&](const instruction_statement::return_inst& inst) -> return_type
    {
        check_arity("return", 1);
        Value* arg = get_value(*begin, inst.type.llvm_type());
        return {builder.CreateRet(arg), none};
    },
    [&](const instruction_statement::cond_branch& inst) -> return_type
    {
        check_arity("cond_branch", 3);
        Type* int1_type = IntegerType::get(builder.getContext(), 1);
        Value* boolean = get_value(*begin, int1_type);
        const ref_symbol& true_block_name = (begin + 1)->cast_else<ref_symbol>([&]
        {
            fatal<id("cond_branch_invalid_block_name")>((begin + 1)->source());
        });
        const ref_symbol& false_block_name = (begin + 2)->cast_else<ref_symbol>([&]
        {
            fatal<id("cond_branch_invalid_block_name")>((begin + 2)->source());
        });
        BranchInst* value = builder.CreateCondBr(boolean, builder.GetInsertBlock(), builder.GetInsertBlock());
        // the two target blocks are set later on, but nullptr is not valid as parameter, so just use current block for now
        incomplete_cond_branch incomplete{value, true_block_name, false_block_name};
        return {value, incomplete_statement{incomplete}};
    },
    [&](const instruction_statement::branch& inst) -> return_type
    {
        check_arity("branch", 1);
        const ref_symbol& block_name = begin->cast_else<ref_symbol>([&]
        {
            fatal<id("cond_branch_invalid_block_name")>(begin->source());
        });
        BranchInst* value = builder.CreateBr(builder.GetInsertBlock());
        incomplete_branch incomplete{value, block_name};
        return {value, incomplete_statement{incomplete}};
    },
    [&](const instruction_statement::phi& inst) -> return_type
    {
        if(begin == end)
            fatal<id("phi_no_arguments")>(instruction.statement.source());
        
        incomplete_phi incomplete{0, {}, instruction.statement};
        for(auto it = begin; it != end; ++it)
        {
            const list_symbol& incoming = it->cast_else<list_symbol>([&]
            {
                fatal<id("phi_invalid_incoming_node")>(it->source());
            });
            if(incoming.size() != 2)
                fatal<id("phi_invalid_incoming_node_size")>(incoming.source());
            
            const ref_symbol& incoming_variable_name = incoming[0].cast_else<ref_symbol>([&]
            {
                fatal<id("phi_invalid_incoming_variable_name")>(incoming[0].source());
            });
            const ref_symbol& incoming_block_name = incoming[1].cast_else<ref_symbol>([&]
            {
                fatal<id("phi_invalid_incoming_block_name")>(incoming[1].source());
            });

            incomplete.incomings.push_back({incoming_variable_name, incoming_block_name});
        }
        
        PHINode* value = builder.CreatePHI(inst.type.llvm_type(), 0);
        incomplete.value = value;
        return {value, incomplete_statement{move(incomplete)}};
        
    },
    [&](const auto& obj) -> return_type
    {
        assert(false);
    });
}

template<class VariableLookupFunctor>
pair<optional<named_value_info>, optional<incomplete_statement>> compile_statement(const symbol& node, VariableLookupFunctor&& lookup_variable, IRBuilder<>& builder)
{
    const list_symbol& statement = node.cast_else<list_symbol>([&]
    {
        fatal<id("invalid_statement")>(node.source());
    });
    if(statement.empty())
        fatal<id("empty_statement")>(statement.source());
    
    const symbol& resolved_first_node = resolve_refs(statement[0]);
    // unresolvable first node is an error
    if(resolved_first_node.is<id_symbol>() && resolved_first_node.cast<id_symbol>().id() == unique_ids::LET)
    {
        // this is a "let" statement
        if(statement.size() < 2)
            fatal<id("missing_variable_name")>(statement.source());
        const ref_symbol& variable_name = statement[1].cast_else<ref_symbol>([&]
        {
            fatal<id("invalid_variable_name")>(statement[1].source());
        });
        list_symbol::const_iterator instr_begin = statement.begin() + 2;
        list_symbol::const_iterator instr_end = statement.end();
        if(instr_begin == instr_end)
            fatal<id("missing_instruction")>(statement.source());
        auto p = compile_instruction_call(instr_begin, instr_end, lookup_variable, builder);
        Value* value = p.first;
        optional<incomplete_statement>& incomplete = p.second;
        return {named_value_info{statement, variable_name, value}, incomplete};
    }
    
    auto p = compile_instruction_call(statement.begin(), statement.end(), lookup_variable, builder);
    optional<incomplete_statement>& incomplete = p.second;
    return {none, std::move(incomplete)};
}


pair<block_info, unique_ptr<BasicBlock>> compile_block(const symbol& block_node, const unordered_map<identifier_id_t, named_value_info>& global_variable_table, compilation_context& context)
{
    const list_symbol& block_definition = block_node.cast_else<list_symbol>([&]()
    {
        fatal<id("invalid_block_definition")>(block_node.source());
    });

    if(block_definition.size() != 2)
        fatal<id("invalid_block_definition_argument_number")>(block_definition.source());
    
    const ref_symbol& block_name = block_definition[0].cast_else<ref_symbol>([&]
    {
        fatal<id("invalid_block_name")>(block_definition[0].source());
    });

    const list_symbol& block_body = block_definition[1].cast_else<list_symbol>([&]
    {
        fatal<id("invalid_block_body")>(block_definition[1].source());
    });
    
    unique_ptr<BasicBlock> block{BasicBlock::Create(context.llvm(), context.to_string(block_name.identifier()))};
    IRBuilder<> builder{block.get()};

    unordered_map<identifier_id_t, named_value_info> local_variable_table;
    auto lookup_variable = [&](identifier_id_t identifier) -> optional<named_value_info>
    {
        // search in global variables
        auto find_it1 = global_variable_table.find(identifier);
        if(find_it1 != global_variable_table.end())
            return find_it1->second;

        // search in local variables
        auto find_it2 = local_variable_table.find(identifier);
        if(find_it2 != local_variable_table.end())
            return find_it2->second;

        return none;
    };

    vector<incomplete_statement> incomplete_statements;
    for(const symbol& s : block_body)
    {
        auto p = compile_statement(s, lookup_variable, builder);
        optional<named_value_info>& value = p.first;
        optional<incomplete_statement>& incomplete = p.second;
        if(value)
        {
            identifier_id_t identifier = value->name.identifier();
            bool was_inserted;
            tie(ignore, was_inserted) = local_variable_table.insert({identifier, move(*value)});
            if(!was_inserted)
                fatal<id("locally_duplicate_variable_name")>(value->name.source());
        }
        if(incomplete)
            incomplete_statements.push_back(std::move(*incomplete));
    }
    
    block_info info{block_name, move(local_variable_table), false, block.get(), std::move(incomplete_statements)};
    return {move(info), move(block)};
}

void compile_body(const symbol& body_node, Function& function, unordered_map<identifier_id_t, named_value_info> parameter_table, compilation_context& context)
{
    const list_symbol& block_list = body_node.cast_else<list_symbol>([&]
    {
        fatal<id("invalid_block_list")>(body_node.source());
    });
    if(block_list.empty())
        fatal<id("empty_body")>(block_list.source());
    
    unordered_map<identifier_id_t, named_value_info>& function_global_variables = parameter_table;
    unordered_map<identifier_id_t, block_info> block_map;

    // check whether variables in this variable_table have already been defined
    auto check_for_duplicates = [&](const unordered_map<identifier_id_t, named_value_info>& variable_table)
    {
        for(const auto& p : variable_table)
        {
            identifier_id_t variable_name = p.first;
            const named_value_info& info = p.second;
            // check in function global variables
            auto global_table_it = function_global_variables.find(variable_name);
            if(global_table_it != function_global_variables.end())
                fatal<id("duplicate_variable_name")>(info.name.source());
            
            // check in other blocks
            for(const auto& other_block_p : block_map)
            {
                const block_info& other_block = other_block_p.second;
                auto table_it = other_block.variable_table.find(variable_name);
                if(table_it != other_block.variable_table.end())
                    fatal<id("duplicate_variable_name")>(info.name.source());
            }
        }
    };
    
    bool is_first_iteration = true;
    for(const symbol& block_node : block_list)
    {
        pair<block_info, unique_ptr<BasicBlock>> block_p = compile_block(block_node, function_global_variables, context);
        block_info& block = block_p.first;
        check_for_duplicates(block.variable_table);
        if(block.llvm_block->getTerminator() == nullptr) // block is not properly terminated or has terminator not at end
            fatal<id("block_invalid_termination")>(block_node.source());

        if(is_first_iteration)
        {
            block.is_entry_block = true;
            function_global_variables.insert(block.variable_table.begin(), block.variable_table.end());
            is_first_iteration = false;
        }
        
        identifier_id_t block_name = block.block_name.identifier();
        function.getBasicBlockList().push_back(block.llvm_block);
        block_p.second.release();
        block_map.insert({block_name, move(block)});
    }

    auto get_block = [&](const ref_symbol& block_name) -> block_info&
    {
        auto block_it = block_map.find(block_name.identifier());
        if(block_it == block_map.end())
            fatal<id("block_not_found")>(block_name.source());
        return block_it->second;
    };
    auto get_non_entry_block = [&](const ref_symbol& block_name) -> block_info&
    {
        block_info& info = get_block(block_name);
        if(info.llvm_block == &function.getBasicBlockList().front())
            fatal<id("branch_to_entry_block")>(block_name.source());
        return info;
    };

    for(auto& p : block_map)
    {
        block_info& binfo = p.second;
        for(incomplete_statement& incomplete : binfo.incomplete_statements)
        {
            visit(incomplete,
            [&](incomplete_cond_branch& cond_branch)
            {
                assert(cond_branch.value->getNumSuccessors() == 2);
                block_info& true_block = get_non_entry_block(cond_branch.true_block_name);
                cond_branch.value->setSuccessor(0, true_block.llvm_block);
                block_info& false_block = get_non_entry_block(cond_branch.false_block_name);
                cond_branch.value->setSuccessor(1, false_block.llvm_block);
            },
            [&](incomplete_branch& branch)
            {
                assert(branch.value->getNumSuccessors() == 1);
                block_info& block = get_non_entry_block(branch.block_name);
                branch.value->setSuccessor(0, block.llvm_block);
            },
            [&](incomplete_phi&)
            {
                // ignore this case for now:
                // can only be handled when predecessors of blocks can be computed, which is after all branches have been set
            });
        }
    }

    for(auto& p : block_map)
    {
        block_info& binfo = p.second;
        for(incomplete_statement& incomplete : binfo.incomplete_statements)
        {
            visit(incomplete, [&](incomplete_phi& phi)
            {
                BasicBlock* parent_block = phi.value->getParent();
                vector<BasicBlock*> predecessors{pred_begin(parent_block), pred_end(parent_block)};
                vector<int8_t> has_incoming_for_predecessor(predecessors.size(), false);
                // has true iff phi node defines an incoming for the corresponding predecessor basic block with the same index
                for(auto& inc : phi.incomings)
                {
                    // get predecessor block and check this
                    block_info& block = get_block(inc.block_name);
                    auto block_it = find(predecessors.begin(), predecessors.end(), block.llvm_block);
                    if(block_it == predecessors.end())
                        fatal<id("phi_incoming_block_not_predecessor")>(inc.block_name.source());
                    size_t predecessor_index = block_it - predecessors.begin();
                    if(has_incoming_for_predecessor[predecessor_index])
                        fatal<id("phi_incoming_block_twice")>(inc.block_name.source());
                    has_incoming_for_predecessor[predecessor_index] = true;

                    auto value_info_it  = block.variable_table.find(inc.variable_name.identifier());
                    if(value_info_it == block.variable_table.end())
                        fatal<id("phi_incoming_variable_not_defined")>(inc.variable_name.source());
                    named_value_info& value = value_info_it->second;
                    if(value.llvm_value->getType() != phi.value->getType())
                        fatal<id("phi_incoming_variable_type_mismatch")>(inc.variable_name.source());

                    phi.value->addIncoming(value.llvm_value, block.llvm_block);
                }
                if(find(has_incoming_for_predecessor.begin(), has_incoming_for_predecessor.end(), false) != has_incoming_for_predecessor.end())
                    fatal<id("phi_missing_incoming_for_predecessor")>(phi.statement.source());
            },
            [&](auto&)
            {
                // has already been done before
            });
        }
    }
}

pair<unique_ptr<Function>, function_info> compile_function(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context)
{
    if(distance(begin, end) != 3)
        fatal<id("invalid_argument_number")>(blank());
    
    unique_ptr<Function> function;
    unordered_map<identifier_id_t, named_value_info> parameter_table;
    tie(function, parameter_table) = compile_signature(*begin, *(begin + 1), context);
    
    const symbol& body_node = *(begin + 2);
    compile_body(body_node, *function, move(parameter_table), context);
    //verifyFunction(*function); not possible because function has to be embedded into module first
    function_info info;
    info.uses_proc_instructions = true; // TODO
    info.uses_macro_instructions = true; // TODO
    return {move(function), move(info)};
}

