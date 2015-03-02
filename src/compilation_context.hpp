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


enum class identifier_ids
  : identifier_id_t
{
    // import/export identifiers
    EXPORT = 1,
    IMPORT,
    FROM,
    DEF,

    FIRST_UNUSED
};


class compilation_context
{
public:
    compilation_context();
    compilation_context(const compilation_context&) = delete;
    
    compilation_context& operator=(const compilation_context&) = delete;

    llvm::LLVMContext& llvm();

    identifier_id_t identifier_id(const std::string& str);
    const std::string& to_string(identifier_id_t);
private:
    std::unordered_map<std::string, identifier_id_t> identifier_table;
};

#endif

