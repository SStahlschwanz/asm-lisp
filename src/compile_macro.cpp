#include "macro_compilation.hpp"
#include "error/compile_macro_exception.hpp"

#include <algorithm>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <memory>

#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>


using std::distance;
using std::forward;
using std::pair;
using std::tie;
using std::ignore;
using std::vector;
using std::unordered_map;
using std::find;
using std::string;
using std::unique_ptr;
using std::move;

using llvm::Value;
using llvm::Function;
using llvm::FunctionType;
using llvm::LLVMContext;
using llvm::Type;
using llvm::BasicBlock;


template<class ExceptionType>
const symbol& resolve_refs_else(const symbol& s, ExceptionType&& exc)
{
    if(s.is<ref_symbol>())
    {
        const ref_symbol& r = s.cast<ref_symbol>();
        if(r.refered() == 0)
            throw forward<ExceptionType>(exc);
        else
            resolve_refs_else(*r.refered(), forward<ExceptionType>(exc));
    }
    else
        return s;
}

struct value_info
{
    Value* llvm_value;
    // const symbol& definition; TODO
};

pair<unique_ptr<Function>, unordered_map<string, value_info>>
        compile_signature(const symbol& params_node, const symbol& return_type_node)
{
    const list_symbol& params_list = params_node.cast_else<list_symbol>(
            macro_invalid_param_list{params_node.source()});
    
    const symbol& resolved_return_symbol = resolve_refs_else(return_type_node,
            identifier_not_defined{return_type_node.source()});
    const type_symbol& return_type = resolved_return_symbol.cast_else<type_symbol>(
            invalid_return_type{resolved_return_symbol.source()});
    
    vector<Type*> arg_types;
    vector<string> arg_names;
    for(const symbol& s : params_list)
    {
        const list_symbol& param_declaration = s.cast_else<list_symbol>(
                invalid_macro_parameter_declaration{s.source()});
        if(param_declaration.size() != 2)
            throw invalid_parameter_declaration_node_number{param_declaration.source()};
        
        const ref_symbol& param_name = param_declaration[0].cast_else<ref_symbol>(
                invalid_parameter_name{param_declaration[0]});
        if(find(arg_names.begin(), arg_names.end(), param_name.identifier()) != arg_names.end())
            throw duplicate_parameter_name{param_name.source()};
        arg_names.push_back(param_name.identifier());

        const symbol& resolved_param_type_symbol = resolve_refs_else(param_declaration[1],
                identifier_not_defined{param_declaration[1]});
        const type_symbol& param_type = resolved_param_type_symbol.cast_else<type_symbol>(
                invalid_parameter_type{resolved_param_type_symbol.source()});
        arg_types.push_back(param_type.llvm_type());
    }
    
    FunctionType* function_type = FunctionType::get(return_type.llvm_type(), arg_types, false);
    unique_ptr<Function> function{Function::Create(function_type, Function::ExternalLinkage)};
    unordered_map<string, value_info> parameter_table;
    
    auto arg_it = function->arg_begin();
    auto name_it = arg_names.begin();
    for( ; arg_it != function->arg_end(); ++arg_it, ++name_it)
    {
        assert(name_it != arg_names.end());
        arg_it->setName(*name_it);
        parameter_table[*name_it] = value_info{arg_it};
    }
    assert(name_it == arg_names.end());

    return {move(function), move(parameter_table)};
}

struct block_info
{
    BasicBlock* llvm_block;
    const ref_symbol& block_name;
    unordered_map<string, value_info> variable_table;
    bool is_entry_block;
};
block_info compile_block(const symbol& block_node, const unordered_map<string, value_info>& global_variables_table,
        compilation_context& context)
{
    const list_symbol& block_definition = block_node.cast_else<list_symbol>(
            invalid_block_definition{block_node.source()});

    if(block_definition.size() != 3)
        throw invalid_block_definiton_argument_number{block_definition.source()};
    
    
}

void compile_body(const symbol& body_node, Function& function, unordered_map<string, value_info> parameter_table,
        compilation_context& context)
{
    const list_symbol& block_list = body_node.cast_else<list_symbol>(
            invalid_block_list{body_node});
    if(block_list.empty())
        throw macro_empty_body{block_list};
    
    unordered_map<string, value_info>& function_global_variables = parameter_table;
    unordered_map<string, block_info> block_map;

    // check whether variables in this variable_table have already been defined
    auto check_for_duplicates = [&](const unordered_map<string, value_info>& variable_table)
    {
        for(const auto& p : variable_table)
        {
            const string& variable_name = p.first;
            // check in function global variables
            auto global_table_it = function_global_variables.find(variable_name);
            if(global_table_it != function_global_variables.end())
                throw duplicate_variable_name{/*TODO*/};
            
            // check in other blocks
            for(const auto& other_block_p : block_map)
            {
                const block_info& other_block = other_block_p.second;
                auto table_it = other_block.variable_table.find(variable_name);
                if(table_it != other_block.variable_table.end())
                    throw duplicate_variable_name{/*TODO*/};
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

macro_symbol compile_macro(list_symbol::const_iterator begin, list_symbol::const_iterator end,
        compilation_context& context)
{
    if(distance(begin, end) != 3)
        throw macro_invalid_argument_number{};
    
    unique_ptr<Function> function;
    unordered_map<string, value_info> parameter_table;
    tie(function, parameter_table) = compile_signature(*begin, *(begin + 1));
    
    const symbol& body_node = *(begin + 2);
    compile_body(body_node, *function, move(parameter_table), context);
}

