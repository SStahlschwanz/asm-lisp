#ifndef MACRO_MODULE_HPP_
#define MACRO_MODULE_HPP_

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>

#include <memory>
#include <utility>

struct macro_module_data_t
{
    llvm::Function* list_create;
};

std::pair<std::unique_ptr<llvm::Module>, macro_module_data_t> create_macro_module(llvm::LLVMContext& llvm_context);

#endif

