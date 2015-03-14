#ifndef COMPILATION_CONTEXT_HPP_
#define COMPILATION_CONTEXT_HPP_

#include "macro_module.hpp"

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


struct module;

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
private:
    std::unique_ptr<macro_execution_environment> macro_env;
    std::unique_ptr<llvm::Module> rt_module;
    std::unique_ptr<module> core;
};

#endif

