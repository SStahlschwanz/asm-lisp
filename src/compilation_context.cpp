#include "compilation_context.hpp"

#include <llvm/IR/LLVMContext.h>

using std::size_t;
using std::unordered_map;
using std::string;

using llvm::LLVMContext;
using llvm::getGlobalContext;

compilation_context::compilation_context()
{
    using namespace identifier_ids;
    identifier_table = 
    {
        {"export", EXPORT},
        {"import", IMPORT},
        {"from", FROM},
        {"def", DEF},
        {"unique", UNIQUE},
        {"int", INT},
    };
    next_uuid = identifier_ids::FIRST_UNUSED;
    assert(identifier_table.size() + 1 == next_uuid);
}

size_t compilation_context::uuid()
{
    return next_uuid++;
}
LLVMContext& compilation_context::llvm()
{
    return getGlobalContext();
}
identifier_id_t compilation_context::identifier_id(const string& str)
{
    identifier_id_t next_identifier_id = identifier_table.size();
    auto it = identifier_table.insert({str, next_identifier_id});
    return it.first->second;
}
