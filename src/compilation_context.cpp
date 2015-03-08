#include "compilation_context.hpp"

#include "macro_module.hpp"

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
{
    identifier_table = 
    {
        {"export", static_cast<size_t>(identifier_ids::EXPORT)},
        {"import", static_cast<size_t>(identifier_ids::IMPORT)},
        {"from", static_cast<size_t>(identifier_ids::FROM)},
        {"def", static_cast<size_t>(identifier_ids::DEF)}
    };
    assert(identifier_table.size() + 1 == static_cast<size_t>(identifier_ids::FIRST_UNUSED));
}
compilation_context::~compilation_context()
{}

LLVMContext& compilation_context::llvm()
{
    return getGlobalContext();
}
Module& compilation_context::llvm_macro_module()
{
    llvm_init();
    return *macro_module;
}
const macro_module_data_t& compilation_context::macro_module_data() const
{
    return macro_module_data_;
}
ExecutionEngine& compilation_context::llvm_execution_engine()
{
    llvm_init();
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

void compilation_context::llvm_init()
{
    if(!macro_module)
    {
        llvm::InitializeNativeTarget();
    
        unique_ptr<Module> macro_module_owner;
        tie(macro_module_owner, macro_module_data_) = create_macro_module(llvm());

        // execution_engine does not need cleanup apparently
        execution_engine = EngineBuilder(macro_module_owner.get()).create();
        assert(execution_engine);
        macro_module = macro_module_owner.release();
    }
}
