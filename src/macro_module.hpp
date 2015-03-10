#ifndef MACRO_MODULE_HPP_
#define MACRO_MODULE_HPP_

#include <memory>
#include <utility>

#include "symbol.hpp"

namespace llvm
{
class Module;
class Function;
class LLVMContext;
class ExecutionEngine;
}

struct macro_execution_environment
{
    llvm::Module& llvm_module;
    llvm::ExecutionEngine& llvm_engine;

    llvm::Function& list_create;
    llvm::Function& list_size;
    llvm::Function& list_set;
    llvm::Function& list_get;
    llvm::Function& list_push;
    llvm::Function& list_pop;
};

macro_execution_environment create_macro_environment(llvm::LLVMContext& llvm_context);
std::pair<any_symbol, std::vector<std::unique_ptr<any_symbol>>> execute_macro(macro_execution_environment& environment, llvm::Function& function, list_symbol::const_iterator args_begin, list_symbol::const_iterator args_end);

#endif

