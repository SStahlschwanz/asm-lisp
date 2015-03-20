#ifndef MACRO_EXECUTION_HPP_
#define MACRO_EXECUTION_HPP_

#include "node.hpp"

#include <cstddef>
#include <utility>
#include <vector>

typedef uint8_t* node_ptr;
typedef node_ptr macro_function(node_ptr);

std::pair<node&, dynamic_graph> execute_macro(macro_function* func, node_range args);

namespace llvm
{
class Type;
class LLVMContext;
}

llvm::Type& llvm_node_type(llvm::LLVMContext&);

#endif

