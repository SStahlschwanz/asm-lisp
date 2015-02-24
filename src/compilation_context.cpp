#include "compilation_context.hpp"

#include <llvm/IR/LLVMContext.h>

using llvm::LLVMContext;
using llvm::getGlobalContext;

LLVMContext& compilation_context::llvm()
{
    return getGlobalContext();
}
