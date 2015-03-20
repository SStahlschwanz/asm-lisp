#ifndef COMPILE_FUNCTION_HPP_
#define COMPILE_FUNCTION_HPP_

#include "compilation_context.hpp"
#include "instruction_types.hpp"
#include "node.hpp"

#include <utility>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>

namespace llvm
{

class Function;
class BasicBlock;

}



struct block_info
{
    const node& block_node;
    const ref_node& block_name;
    std::unordered_map<std::string, named_value_info> variable_table;
    std::vector<statement> statements;
    llvm::BasicBlock& llvm_block;
};
struct function_info
{
    std::vector<block_info> blocks;
    bool is_ct_only;
    bool is_rt_only;

    llvm::Function& llvm_function;
};


std::pair<std::unique_ptr<llvm::Function>, std::unordered_map<std::string, named_value_info>> compile_signature(const node& params_node, const node& return_type_node, compilation_context& context);

block_info compile_block(const node& block_node, llvm::BasicBlock& llvm_block, std::function<named_value_info* (const ref_node&)> lookup_global_variable, compilation_context& context);

std::pair<std::unique_ptr<llvm::Function>, function_info> compile_function(node_range source_range, compilation_context& context);

macro_node compile_macro(node_range source, compilation_context& context);
proc_node compile_proc(node_range source, compilation_context& context);

#endif

