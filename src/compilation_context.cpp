#include "compilation_context.hpp"

#include "macro_module.hpp"
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
    identifier_table = 
    {
        {"export", static_cast<size_t>(identifier_ids::EXPORT)},
        {"import", static_cast<size_t>(identifier_ids::IMPORT)},
        {"from", static_cast<size_t>(identifier_ids::FROM)},
        {"def", static_cast<size_t>(identifier_ids::DEF)},
        {"core", static_cast<size_t>(identifier_ids::CORE)}
    };
    assert(identifier_table.size() + 1 == static_cast<size_t>(identifier_ids::FIRST_UNUSED));
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
identifier_id_t compilation_context::identifier_id(const string& str)
{
    // identifier ids start with index 1
    identifier_id_t next_identifier_id = identifier_table.size() + 1;
    auto it = identifier_table.insert({str, next_identifier_id});
    return it.first->second;
}
const string& compilation_context::to_string(identifier_id_t identifier_id)
{
    for(const pair<const string, identifier_id_t>& p : identifier_table)
    {
        if(p.second == identifier_id)
            return p.first;
    }
    assert(false);
    return *static_cast<string*>(nullptr); // suppress warnings
}
module& compilation_context::core_module()
{
    return *core;
}
