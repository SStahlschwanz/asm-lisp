#include "compilation_context.hpp"

#include <llvm/IR/LLVMContext.h>

using std::size_t;
using std::unordered_map;
using std::string;
using std::pair;

using llvm::LLVMContext;
using llvm::getGlobalContext;

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

LLVMContext& compilation_context::llvm()
{
    return getGlobalContext();
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

