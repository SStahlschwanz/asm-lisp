#include "compile_function.hpp"

#include "error/compile_function_error.hpp"

#include <llvm/IR/CFG.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace compile_function_error;

using llvm::Function;
using llvm::Type;
using llvm::IntegerType;
using llvm::FunctionType;
using llvm::Value;
using llvm::Argument;
using llvm::BasicBlock;
using llvm::BranchInst;
using llvm::IRBuilder;
using llvm::pred_begin;
using llvm::pred_end;
using llvm::ValueToValueMapTy;

using boost::blank;
using boost::get;

using std::pair;
using std::tuple;
using std::unique_ptr;
using std::make_shared;
using std::unordered_map;
using std::string;
using std::move;
using std::vector;
using std::ignore;

pair<unique_ptr<Function>, unordered_map<string, named_value_info>> compile_signature(const node& params_node, const node& return_type_node, compilation_context& context)
{
    const list_node& params_list = params_node.cast_else<list_node>([&]
    {
        fatal<id("invalid_param_list")>(params_node.source());
    });
    
    type_info return_type = compile_type(return_type_node, context.llvm());

    for_each(params_list, [&](const node& param_declaration_node)
    {
        if(!param_declaration_node.is<list_node>())
            fatal<id("invalid_parameter_declaration")>(param_declaration_node.source());
    });
    auto param_declarations_range = mapped(params_list,
    [&](const node& n) -> const list_node&
    {
        return n.cast<list_node>();
    });
    for_each(param_declarations_range, [&](const list_node& param_declaration)
    {
        if(param_declaration.size() != 2)
            fatal<id("invalid_parameter_declaration_node_number")>(param_declaration.source());
    });

    auto arg_types = save<vector<Type*>>(mapped(param_declarations_range,
    [&](const list_node& param_declaration) -> Type*
    {
        type_info param_type = compile_type(param_declaration[1], context.llvm());
        return &param_type.llvm_type;
    }));

    FunctionType* function_type = FunctionType::get(&return_type.llvm_type, arg_types, false);
    unique_ptr<Function> function{Function::Create(function_type, Function::InternalLinkage)};

    unordered_map<string, named_value_info> parameter_table;
    auto args_range = rangeify(function->arg_begin(), function->arg_end());
    for_each(zipped(param_declarations_range, args_range), unpacking(
    [&](const list_node& param_declaration, Argument& arg)
    {
        const ref_node& name_ref = param_declaration[0].cast_else<ref_node>([&]
        {
            fatal<id("invalid_parameter_name")>(param_declaration[0].source()); 
        });

        bool was_inserted;
        tie(ignore, was_inserted) = parameter_table.insert({save<string>(name_ref.identifier()), named_value_info{arg, name_ref}});
        if(!was_inserted)
            fatal<id("duplicate_parameter_name")>(name_ref.source());
    }));

    return {move(function), move(parameter_table)};
}


template<class LookupVariableFunctor>
block_info compile_block(const node& block_node, BasicBlock& llvm_block, LookupVariableFunctor&& lookup_global_variable, compilation_context& context)
{
    const list_node& block_definition = block_node.cast_else<list_node>([&]()
    {
        fatal<id("invalid_block_definition")>(block_node.source());
    });

    if(block_definition.size() != 2)
        fatal<id("invalid_block_definition_argument_number")>(block_definition.source());
    
    const ref_node& block_name = block_definition[0].cast_else<ref_node>([&]
    {
        fatal<id("invalid_block_name")>(block_definition[0].source());
    });

    const list_node& block_body = block_definition[1].cast_else<list_node>([&]
    {
        fatal<id("invalid_block_body")>(block_definition[1].source());
    });
    
    llvm_block.setName(save<string>(block_name.identifier()));
    IRBuilder<> builder{&llvm_block};

    unordered_map<string, named_value_info> local_variable_table;
    vector<statement> statements;
    statements.reserve(block_body.size());

    auto define_variable = [&](const ref_node& name, const named_value_info& value_info)
    {
        local_variable_table.insert({save<string>(name.identifier()), value_info});
    };
    auto lookup_variable = [&](const ref_node& name) -> named_value_info&
    {
        named_value_info* value = lookup_global_variable(name);
        if(value)
            return *value;
        auto find_it = local_variable_table.find(save<string>(name.identifier()));
        if(find_it == local_variable_table.end())
            fatal<id("variable_undefined")>(name.source());

        return find_it->second;
    };
    auto add_statement = [&](const node& statement_node, const instruction_data& data)
    {
        pair<const node&, instruction_data> statement{statement_node, data};
        statements.push_back(move(statement));
    };

    auto st_context = make_statement_context(builder, define_variable, lookup_variable, add_statement, context.macro_environment());
    for(const node& n : block_body)
        compile_statement(n, st_context);

    return {block_node, block_name, move(local_variable_table), move(statements), llvm_block};
}

block_info compile_block(const node& block_node, BasicBlock& llvm_block, std::function<named_value_info* (const ref_node&)> lookup_global_variable, compilation_context& context)
{
    auto lookup_global_variable_proxy = [&](const ref_node& name_ref)
    {
        return lookup_global_variable(name_ref);
    };
    return compile_block(block_node, llvm_block, lookup_global_variable_proxy, context);
}

pair<unique_ptr<Function>, function_info> compile_function(node_range source_range, compilation_context& context)
{
    if(length(source_range) != 3)
        fatal<id("invalid_argument_number")>(blank());

    const node& parameters_node = source_range.front();
    source_range.pop_front();
    const node& return_type_node = source_range.front();
    source_range.pop_front();
    const node& body_node = source_range.front();
    source_range.pop_front();

    unique_ptr<Function> function;
    unordered_map<string, named_value_info> parameter_table;
    tie(function, parameter_table) = compile_signature(parameters_node, return_type_node, context);
    
    vector<block_info> blocks;

    auto lookup_global_variable = [&](const ref_node& name_ref) -> named_value_info*
    {
        string name = save<string>(name_ref.identifier());
        if(!blocks.empty())
        {
            auto initial_block_it = blocks.front().variable_table.find(name);
            if(initial_block_it != blocks.front().variable_table.end())
                return &initial_block_it->second;
        }
        auto params_it = parameter_table.find(name);
        if(params_it != parameter_table.end())
            return &params_it->second;

        return nullptr;
        //fatal<id("variable_undefined")>(name_ref.source());
    };
    auto check_for_duplicates = [&](const unordered_map<string, named_value_info>& variable_table)
    {
        for(const auto& p : variable_table)
        {
            const string variable_name = p.first;
            const named_value_info& info = p.second;

            // check in function global variables
            auto param_table_it = parameter_table.find(variable_name);
            if(param_table_it != parameter_table.end())
                fatal<id("globally_duplicate_variable_name")>(info.name_ref.source());
            
            // check in other blocks
            for(const block_info& other_block : blocks)
            {
                auto block_table_it = other_block.variable_table.find(variable_name);
                if(block_table_it != other_block.variable_table.end())
                    fatal<id("globally_duplicate_variable_name")>(info.name_ref.source());
            }
        }
    };

    const list_node& blocks_list = body_node.cast_else<list_node>([&]
    {
        fatal<id("invalid_block_list")>(body_node.source());
    });

    for(const node& block_node : blocks_list)
    {
        BasicBlock& llvm_block = *BasicBlock::Create(context.llvm(), "", function.get());
        block_info info = compile_block(block_node, llvm_block, lookup_global_variable, context);
        check_for_duplicates(info.variable_table);
        blocks.push_back(move(info));
    }

    auto get_block = [&](const ref_node& name_ref) -> block_info&
    {
        auto it = find_if(blocks.begin(), blocks.end(), [&](block_info& block)
        {
            return block.block_name.identifier() == name_ref.identifier();
        });
        if(it == blocks.end())
            fatal<id("block_not_found")>(name_ref.source());
        return *it;
    };

    for(block_info& block : blocks)
    {
        statement& last_statement = block.statements.back();
        if(instruction::branch* br = get<instruction::branch>(&last_statement.second))
        {
            block_info& branch_block = get_block(br->block_name);
            br->value = BranchInst::Create(&branch_block.llvm_block);
            block.llvm_block.getInstList().push_back(br->value);
        }
        else if(instruction::cond_branch* cond_br = get<instruction::cond_branch>(&last_statement.second))
        {
            block_info& true_block = get_block(cond_br->true_block_name);
            block_info& false_block = get_block(cond_br->false_block_name);
            cond_br->value = BranchInst::Create(&true_block.llvm_block, &false_block.llvm_block, &cond_br->boolean, &block.llvm_block);
        }

        if(block.llvm_block.getTerminator() == nullptr)
            fatal<id("block_invalid_termination")>(block.block_node.source());
    }

    bool is_ct_only = false;
    bool is_rt_only = false;
    for(block_info& block : blocks)
    {
        for(statement& st : block.statements)
        {
            if(instruction::phi* phi = get<instruction::phi>(&st.second))
            {
                BasicBlock& parent_block = *phi->llvm_value.getParent();
                vector<BasicBlock*> predecessors{pred_begin(&parent_block), pred_end(&parent_block)};
                vector<int8_t> has_incoming_for_predecessor(predecessors.size(), false);
                for(instruction::phi::incoming& inc : phi->incomings)
                {
                    block_info& block = get_block(inc.block_name);
                    auto block_it = find(predecessors.begin(), predecessors.end(), &block.llvm_block);
                    if(block_it == predecessors.end())
                        fatal<id("phi_incoming_block_not_predecessor")>(inc.block_name.source());

                    size_t predecessor_index = block_it - predecessors.begin();
                    if(has_incoming_for_predecessor[predecessor_index])
                        fatal<id("phi_incoming_block_twice")>(inc.block_name.source());
                    has_incoming_for_predecessor[predecessor_index] = true;

                    auto value_info_it  = block.variable_table.find(save<string>(inc.variable_name.identifier()));
                    if(value_info_it == block.variable_table.end())
                        fatal<id("phi_incoming_variable_not_defined")>(inc.variable_name.source());
                    named_value_info& value = value_info_it->second;
                    if(value.llvm_value.getType() != phi->llvm_value.getType())
                        fatal<id("phi_incoming_variable_type_mismatch")>(inc.variable_name.source());

                    phi->llvm_value.addIncoming(&value.llvm_value, &block.llvm_block);
                }
                for(int8_t has_incoming : has_incoming_for_predecessor)
                {
                    if(!has_incoming)
                        fatal<id("phi_missing_incoming_for_predecessor")>(st.first.source());
                }
            }


            is_ct_only = is_ct_only || visit<bool>(st.second, [&](const auto& inst)
            {
                return inst.is_ct_only;
            });
            is_rt_only = is_rt_only || visit<bool>(st.second, [&](const auto& inst)
            {
                return inst.is_rt_only;
            });
        }
    }

    Function& func = *function;
    return {move(function), function_info{move(blocks), is_ct_only, is_rt_only, func}};
}

macro_node compile_macro(node_range source, compilation_context& context)
{
    auto p = compile_function(source, context);
    unique_ptr<Function>& func_owner = p.first;
    function_info& func_info = p.second;
    
    Type* symbol_index_type = IntegerType::get(context.llvm(), 64);
    Type* macro_type = FunctionType::get(symbol_index_type, vector<Type*>{symbol_index_type}, false);
    
    if(macro_type != func_info.llvm_function.getFunctionType())
        fatal<id("invalid_macro_signature")>(boost::blank());
    if(func_info.is_rt_only)
        fatal<id("macro_uses_rt_only_instruction")>(blank());

    context.macro_environment().llvm_module.getFunctionList().push_back(func_owner.get());
    func_owner.release();

    typedef uint64_t macro_function_signature(uint64_t);
    auto func_ptr = (macro_function_signature*) context.macro_environment().llvm_engine.getPointerToFunction(&func_info.llvm_function);
    assert(func_ptr);

    auto macro_func = [func_ptr](node_range nodes) -> pair<node&, dynamic_graph>
    {
        // TODO
    };

    return {make_shared<std::function<macro_node::macro>>(macro_func)};
}

proc_node compile_proc(node_range source, compilation_context& context)
{
    auto p = compile_function(source, context);
    unique_ptr<Function>& func_owner = p.first;
    function_info& func_info = p.second;

    Function* ct_function = nullptr;
    Function* rt_function = nullptr;

    if(!func_info.is_rt_only)
    {
        ValueToValueMapTy vtvm;
        unique_ptr<Function> cloned_func{CloneFunction(func_owner.get(), vtvm, false)};
        for(block_info& block : func_info.blocks)
        {
            for(statement& st : block.statements)
            {
                if(auto call = get<instruction::call>(&st.second))
                    call->llvm_value.setCalledFunction(call->callee.ct_function());
            }
        }
        context.macro_environment().llvm_module.getFunctionList().push_back(cloned_func.get());
        ct_function = cloned_func.release();
    }
    if(!func_info.is_ct_only)
    {
        ValueToValueMapTy vtvm;
        unique_ptr<Function> cloned_func{CloneFunction(func_owner.get(), vtvm, false)};
        for(block_info& block : func_info.blocks)
        {
            for(statement& st : block.statements)
            {
                if(auto call = get<instruction::call>(&st.second))
                    call->llvm_value.setCalledFunction(call->callee.rt_function());
            }
        }
        context.runtime_module().getFunctionList().push_back(cloned_func.get());
        rt_function = cloned_func.release();
    }

    if(ct_function == nullptr && rt_function == nullptr)
        fatal<id("proc_neither_ct_nor_rt")>(blank());

    return proc_node{ct_function, rt_function};
}

