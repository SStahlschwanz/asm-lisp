#include "compilation_context.hpp"

#include "core_module.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>

using std::size_t;
using std::unordered_map;
using std::string;
using std::pair;
using std::make_unique;
using std::unique_ptr;

using llvm::LLVMContext;
using llvm::getGlobalContext;
using llvm::Module;
using llvm::ExecutionEngine;
using llvm::EngineBuilder;

compilation_context::compilation_context()
  : rt_module{new Module{"runtime module", llvm()}}
{
    core = make_unique<module>(create_core_module(*this));
}
compilation_context::~compilation_context()
{}

LLVMContext& compilation_context::llvm()
{
    return getGlobalContext();
}
macro_execution_environment& compilation_context::macro_environment()
{
    if(!macro_env)
    {
        llvm::InitializeNativeTarget();
        macro_env = make_unique<macro_execution_environment>(create_macro_environment(llvm()));
    }
    return *macro_env;
}
Module& compilation_context::runtime_module()
{
    return *rt_module;
}
module& compilation_context::core_module()
{
    return *core;
}
