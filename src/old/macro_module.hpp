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

    llvm::Function& is_id;
    llvm::Function& is_lit;
    llvm::Function& is_ref;
    llvm::Function& is_list;
    llvm::Function& is_macro;

    llvm::Function& lit_create;
    llvm::Function& lit_size;
    llvm::Function& lit_set;
    llvm::Function& lit_get;
    llvm::Function& lit_push;
    llvm::Function& lit_pop;

    llvm::Function& list_create;
    llvm::Function& list_size;
    llvm::Function& list_set;
    llvm::Function& list_get;
    llvm::Function& list_push;
    llvm::Function& list_pop;
};

macro_execution_environment create_macro_environment(llvm::LLVMContext& llvm_context);

#endif

