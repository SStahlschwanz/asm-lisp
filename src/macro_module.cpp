#include "macro_module.hpp"

#include <memory>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>


using std::unique_ptr;
using std::make_unique;
using std::vector;
using std::pair;
using std::move;

using llvm::LLVMContext;
using llvm::Function;
using llvm::Module;
using llvm::FunctionType;
using llvm::IntegerType;
using llvm::Type;


pair<unique_ptr<Module>, macro_module_data_t> create_macro_module(LLVMContext& llvm_context)
{
    auto module = make_unique<Module>("macro module", llvm_context);

    Type* llvm_int64 = IntegerType::get(llvm_context, 64);
    vector<Type*> params_symbol{1, llvm_int64};
    auto signature_symbol_symbol = FunctionType::get(llvm_int64, params_symbol, false);
    
    macro_module_data_t data;
    data.list_create = Function::Create(signature_symbol_symbol, Function::InternalLinkage, "macro_create_list", module.get());
    
    return {move(module), data};
}

struct macro_execution_environment
{
};

extern "C"
{

uint64_t macro_create_list(uint64_t)
{
    return 43;
}

}

