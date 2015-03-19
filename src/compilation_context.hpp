#ifndef COMPILATION_CONTEXT_HPP_
#define COMPILATION_CONTEXT_HPP_

#include "macro_environment.hpp"

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

    CORE,

    FIRST_UNUSED
};

struct module;
struct macro_execution_environment;

class compilation_context
{
public:
    compilation_context();
    compilation_context(const compilation_context&) = delete;
    ~compilation_context();
    
    compilation_context& operator=(const compilation_context&) = delete;

    llvm::LLVMContext& llvm();
    macro_execution_environment& macro_environment();
    llvm::Module& runtime_module();
    module& core_module();

    identifier_id_t identifier_id(const std::string& str);
    const std::string& to_string(identifier_id_t);
private:
    std::unique_ptr<macro_execution_environment> macro_env;
    std::unique_ptr<llvm::Module> rt_module;
    std::unique_ptr<module> core;
};

#endif

