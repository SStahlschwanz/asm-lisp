#include "compilation_context.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

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
{
    identifier_table = 
    {
        {"export", static_cast<size_t>(identifier_ids::EXPORT)},
        {"import", static_cast<size_t>(identifier_ids::IMPORT)},
        {"from", static_cast<size_t>(identifier_ids::FROM)},
        {"def", static_cast<size_t>(identifier_ids::DEF)}
    };
    assert(identifier_table.size() + 1 == static_cast<size_t>(identifier_ids::FIRST_UNUSED));
    
    auto macro_module_owner = make_unique<Module>("macro module", llvm());
    macro_module = macro_module_owner.get();
    //execution_engine = unique_ptr<ExecutionEngine>{EngineBuilder(macro_module).create()};
    //assert(execution_engine);
    macro_module_owner.release();
}
compilation_context::~compilation_context()
{}

LLVMContext& compilation_context::llvm()
{
    return getGlobalContext();
}
Module& compilation_context::llvm_macro_module()
{
    return *macro_module;
}
ExecutionEngine& compilation_context::llvm_execution_engine()
{
    return *execution_engine;
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

