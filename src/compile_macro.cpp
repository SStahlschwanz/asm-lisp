#include "compile_macro.hpp"
#include "error/compile_macro_exception.hpp"
#include "core_unique_ids.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>

#include <algorithm>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>


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

using llvm::Value;
using llvm::Function;
using llvm::FunctionType;
using llvm::LLVMContext;
using llvm::Type;
using llvm::BasicBlock;
using llvm::isa;
using llvm::IntegerType;

using boost::get;

using namespace core_exception;
using namespace compile_macro_exception;

template<class ExceptionType>
const symbol& resolve_refs_else(const symbol& s, ExceptionType&& exc)
{
    if(s.is<ref_symbol>())
    {
        const ref_symbol& r = s.cast<ref_symbol>();
        if(r.refered() == 0)
            throw forward<ExceptionType>(exc);
        else
            return resolve_refs_else(*r.refered(), forward<ExceptionType>(exc));
    }
    else
        return s;
}
const symbol& resolve_refs(const symbol& s)
{
    return resolve_refs_else(s, identifier_not_defined{s.source()});
}


pair<unique_ptr<Function>, unordered_map<identifier_id_t, value_info>> compile_signature(const symbol& params_node, const symbol& return_type_node, compilation_context& context)
{
    const list_symbol& params_list = params_node.cast_else<list_symbol>([&]()
    {
        throw invalid_param_list{params_node.source()};
    });
    
    const symbol& resolved_return_symbol = resolve_refs(return_type_node);
    const type_symbol& return_type = resolved_return_symbol.cast_else<type_symbol>([&]()
    {
        throw invalid_return_type{resolved_return_symbol.source()};
    });
    
    vector<value_info> arg_infos;
    for(const symbol& s : params_list)
    {
        const list_symbol& param_declaration = s.cast_else<list_symbol>([&]()
        {
            throw invalid_parameter_declaration{s.source()};
        });
        if(param_declaration.size() != 2)
            throw invalid_parameter_declaration_node_number{param_declaration.source()};
        
        const ref_symbol& param_name = param_declaration[0].cast_else<ref_symbol>([&]()
        {
            throw invalid_parameter_name{param_declaration[0].source()};
        });

        const symbol& resolved_param_type_symbol = resolve_refs(param_declaration[1]);
        const type_symbol& param_type = resolved_param_type_symbol.cast_else<type_symbol>([&]()
        {
            throw invalid_parameter_type{resolved_param_type_symbol.source()};
        });
        arg_infos.push_back(value_info{param_declaration, param_name, param_type, 0});
    }
    
    vector<Type*> arg_types;
    arg_types.reserve(arg_infos.size());
    for(value_info& info : arg_infos)
        arg_types.push_back(info.type.llvm_type());
    
    FunctionType* function_type = FunctionType::get(return_type.llvm_type(), arg_types, false);
    unique_ptr<Function> function{Function::Create(function_type, Function::ExternalLinkage)};
    unordered_map<identifier_id_t, value_info> parameter_table;
    
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
            throw duplicate_parameter_name{info_it->name.source()};
    }
    assert(info_it == arg_infos.end());

    return {move(function), move(parameter_table)};
}

template<class InstructionType>
instruction_statement compile_unary_typed_instruction(const char* name, const list_symbol& statement)
{
    if(statement.size() != 2)
        throw instruction_constructor_invalid_argument_number{statement.source(), name, 1, statement.size() - 1};

    const symbol& resolved_param = resolve_refs(statement[1]);
    const type_symbol& type = resolved_param.cast_else<type_symbol>([&]
    {
        throw invalid_instruction_type_parameter{resolved_param.source()};
    });
    return instruction_statement{statement, InstructionType{type}};
}
template<class InstructionType>
instruction_statement compile_number_instruction(const char* name, const list_symbol& statement)
{
    instruction_statement result = compile_unary_typed_instruction<InstructionType>(name, statement);
    const type_symbol& type = get<InstructionType>(result.instruction).type;
    if(!isa<IntegerType>(type.llvm_type()))
        throw invalid_number_type{type.source()};
    return result;
}
instruction_statement compile_cmp_instruction(const list_symbol& statement)
{
    if(statement.size() != 3)
        throw instruction_constructor_invalid_argument_number{statement.source(), "cmp", 2, statement.size() - 1};

    const symbol& resolved_first_arg = resolve_refs(statement[1]);
    const symbol& resolved_second_arg = resolve_refs(statement[2]);
    
    const id_symbol& first_arg = resolved_first_arg.cast_else<id_symbol>([&]
    {
        throw invalid_comparison_kind{resolved_first_arg.source()};
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
        throw unknown_comparison_kind{resolved_first_arg.source()};

    const type_symbol& second_arg = resolved_second_arg.cast_else<type_symbol>([&]
    {
        throw invalid_instruction_type_parameter{resolved_second_arg.source()};
    });

    return instruction_statement{statement, instruction_statement::cmp{comparison_kind, second_arg}};
}

instruction_statement compile_instruction(const symbol& node)
{
    if(node.is<list_symbol>())
    {
        const list_symbol& statement = node.cast<list_symbol>();
        if(statement.empty())
            throw empty_instruction{statement.source()};
        const symbol& resolved_instruction_constructor = resolve_refs(statement[0]);
        const id_symbol& instruction_constructor = resolved_instruction_constructor.cast_else<id_symbol>([&]
        {
            throw invalid_instruction_constructor{statement[0].source()};
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
                throw instruction_constructor_invalid_argument_number{statement.source(), "cond_branch", 0, statement.size() - 1};
            return instruction_statement{statement, instruction_statement::cond_branch{}};
        case unique_ids::CMP:
            return compile_cmp_instruction(statement);
        case unique_ids::CALL:
            throw not_implemented{"instruction"};
        default:
            throw unknown_instruction_constructor{instruction_constructor.source()};
        }
    }
}
/*
struct block_info
{
    const ref_symbol& block_name;
    unordered_map<identifier_id_t, value_info> variable_table;
    bool is_entry_block;
    BasicBlock* llvm_block;
};



block_info compile_block(const symbol& block_node, const unordered_map<string, value_info>& global_variables_table,
        compilation_context& context)
{
    const list_symbol& block_definition = block_node.cast_else<list_symbol>([&]()
    {
        throw invalid_block_definition{block_node.source()};
    });

    if(block_definition.size() != 3)
        throw invalid_block_definition_argument_number{block_definition.source()};
    
    
}

void compile_body(const symbol& body_node, Function& function, unordered_map<string, value_info> parameter_table, compilation_context& context)
{
    const list_symbol& block_list = body_node.cast_else<list_symbol>(
            invalid_block_list{body_node});
    if(block_list.empty())
        throw empty_body{block_list};
    
    unordered_map<identifier_id_t, value_info>& function_global_variables = parameter_table;
    unordered_map<string, block_info> block_map;

    // check whether variables in this variable_table have already been defined
    auto check_for_duplicates = [&](const unordered_map<string, value_info>& variable_table)
    {
        for(const auto& p : variable_table)
        {
            identifier_id_t identifier = p.first;
            // check in function global variables
            auto global_table_it = function_global_variables.find(variable_name);
            if(global_table_it != function_global_variables.end())
                throw duplicate_variable_name{};
            
            // check in other blocks
            for(const auto& other_block_p : block_map)
            {
                const block_info& other_block = other_block_p.second;
                auto table_it = other_block.variable_table.find(variable_name);
                if(table_it != other_block.variable_table.end())
                    throw duplicate_variable_name{};
            }
        }
    };

    auto block_node_it = block_list.begin();
    block_info entry_block = compile_block(*block_node_it, function_global_variables, context);
    check_for_duplicates(entry_block.variable_table);
    function_global_variables.insert(entry_block.variable_table.begin(), entry_block.variable_table.end());
    entry_block.is_entry_block = true;
    const string& first_block_name = entry_block.block_name.identifier();
    block_map.insert({first_block_name, move(entry_block)});
    ++block_node_it;

    for( ; block_node_it != block_list.end(); ++block_node_it)
    {
        block_info block = compile_block(*block_node_it, function_global_variables, context);
        check_for_duplicates(block.variable_table);
        block.is_entry_block = false;
        bool was_inserted;
        tie(ignore, was_inserted) = block_map.insert({block.block_name.identifier(), move(block)});
        if(!was_inserted)
            throw duplicate_block_name{block.block_name.source()};
    }
}
*/

macro_symbol compile_macro(list_symbol::const_iterator begin, list_symbol::const_iterator end, compilation_context& context)
{
    if(distance(begin, end) != 3)
        throw invalid_argument_number{};
    
    unique_ptr<Function> function;
    unordered_map<identifier_id_t, value_info> parameter_table;
    tie(function, parameter_table) = compile_signature(*begin, *(begin + 1), context);
    
    //const symbol& body_node = *(begin + 2);
    //compile_body(body_node, *function, move(parameter_table), context);
}

