#ifndef COMPILATION_CONTEXT_HPP_
#define COMPILATION_CONTEXT_HPP_

#include <cstddef>
#include <string>
#include <unordered_map>
#include <memory>

namespace llvm
{
class LLVMContext;
class Module;
class ExecutionEngine;
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
    ~compilation_context();
    
    compilation_context& operator=(const compilation_context&) = delete;

    llvm::LLVMContext& llvm();
    llvm::Module& llvm_macro_module();
    llvm::ExecutionEngine& llvm_execution_engine();

    identifier_id_t identifier_id(const std::string& str);
    const std::string& to_string(identifier_id_t);
private:
    std::unordered_map<std::string, identifier_id_t> identifier_table;
    //std::unique_ptr<llvm::Module> macro_module;
    llvm::Module* macro_module;
    llvm::ExecutionEngine* execution_engine;

    void llvm_init();
};

#endif

