#ifndef COMPILATION_CONTEXT_HPP_
#define COMPILATION_CONTEXT_HPP_

#include <cstddef>
#include <string>
#include <unordered_map>

namespace llvm
{
class LLVMContext;
}

typedef size_t identifier_id_t;

namespace identifier_ids
{
// import/export identifiers
constexpr identifier_id_t EXPORT = 1;
constexpr identifier_id_t IMPORT = 2;
constexpr identifier_id_t FROM = 3;
// def
constexpr identifier_id_t DEF = 4;
// core identifiers
constexpr identifier_id_t UNIQUE = 5;
constexpr identifier_id_t INT = 6;

constexpr identifier_id_t FIRST_UNUSED = 7;
}

class compilation_context
{
public:
    compilation_context();
    
    std::size_t uuid();
    llvm::LLVMContext& llvm();

    identifier_id_t identifier_id(const std::string& str);
private:
    std::size_t next_uuid;
    std::unordered_map<std::string, identifier_id_t> identifier_table;
};

#endif

