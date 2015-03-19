#include "macro_environment.hpp"
#include "compilation_context.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>

#include <boost/variant.hpp>

#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <setjmp.h>

using std::unique_ptr;
using std::make_unique;
using std::vector;
using std::pair;
using std::move;
using std::string;
using std::vector;
using std::size_t;

using llvm::LLVMContext;
using llvm::Function;
using llvm::Module;
using llvm::ExecutionEngine;
using llvm::EngineBuilder;
using llvm::FunctionType;
using llvm::IntegerType;
using llvm::Type;

macro_execution_environment create_macro_environment(llvm::LLVMContext& llvm_context)
{
    auto module_owner = make_unique<Module>("macro module", llvm_context);
    ExecutionEngine* engine = EngineBuilder(module_owner.get()).create();
    assert(engine);
    Module* module = module_owner.release();
    
    Type* int64 = IntegerType::get(llvm_context, 64);
    Type* int8 = IntegerType::get(llvm_context, 8);
    Type* int1 = IntegerType::get(llvm_context, 1);

    Type* llvm_void = Type::getVoidTy(llvm_context);

    vector<Type*> params = {};
    FunctionType* sig_int64 = FunctionType::get(int64, params, false);

    params = {int64};
    FunctionType* sig_int64_int64 = FunctionType::get(int64, params, false);
    FunctionType* sig_void_int64 = FunctionType::get(llvm_void, params, false);
    FunctionType* sig_int1_int64 = FunctionType::get(int1, params, false);

    params = {int64, int64};
    FunctionType* sig_int64_2int64 = FunctionType::get(int64, params, false);
    FunctionType* sig_void_2int64 = FunctionType::get(llvm_void, params, false);
    FunctionType* sig_int8_2int64 = FunctionType::get(int8, params, false);

    params = {int64, int64, int64};
    FunctionType* sig_void_3int64 = FunctionType::get(int64, params, false);

    params = {int64, int64, int8};
    FunctionType* sig_void_2int64_int8 = FunctionType::get(llvm_void, params, false);

    params = {int64, int8};
    FunctionType* sig_void_int64_int8 = FunctionType::get(llvm_void, params, false);


    Function* is_id = Function::Create(sig_int1_int64, Function::InternalLinkage, "macro_is_id", module);
    Function* is_lit = Function::Create(sig_int1_int64, Function::InternalLinkage, "macro_is_lit", module);
    Function* is_ref = Function::Create(sig_int1_int64, Function::InternalLinkage, "macro_is_ref", module);
    Function* is_list = Function::Create(sig_int1_int64, Function::InternalLinkage, "macro_is_list", module);
    Function* is_macro = Function::Create(sig_int1_int64, Function::InternalLinkage, "macro_is_macro", module);

    Function* lit_create = Function::Create(sig_int64, Function::InternalLinkage, "macro_lit_create", module);
    Function* lit_size = Function::Create(sig_int64_int64, Function::InternalLinkage, "macro_lit_size", module);
    Function* lit_get = Function::Create(sig_int8_2int64, Function::InternalLinkage, "macro_lit_get", module);
    Function* lit_set = Function::Create(sig_void_2int64_int8, Function::InternalLinkage, "macro_lit_set", module);
    Function* lit_push = Function::Create(sig_void_int64_int8, Function::InternalLinkage, "macro_lit_push", module);
    Function* lit_pop = Function::Create(sig_void_int64, Function::InternalLinkage, "macro_lit_pop", module);
    
    Function* list_create = Function::Create(sig_int64, Function::InternalLinkage, "macro_list_create", module);
    Function* list_size = Function::Create(sig_int64_int64, Function::InternalLinkage, "macro_list_size", module);
    Function* list_get = Function::Create(sig_int64_2int64, Function::InternalLinkage, "macro_list_get", module);
    Function* list_set = Function::Create(sig_void_3int64, Function::InternalLinkage, "macro_list_set", module);
    Function* list_push = Function::Create(sig_void_2int64, Function::InternalLinkage, "macro_list_push", module);
    Function* list_pop = Function::Create(sig_void_int64, Function::InternalLinkage, "macro_list_pop", module);

    return macro_execution_environment
    {
        *module,
        *engine,

        *is_id,
        *is_lit,
        *is_ref,
        *is_list,
        *is_macro,

        *lit_create,
        *lit_size,
        *lit_set,
        *lit_get,
        *lit_push,
        *lit_pop,

        *list_create,
        *list_size,
        *list_set,
        *list_get,
        *list_push,
        *list_pop
    };
}

