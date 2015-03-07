#ifndef COMPILE_TYPE_HPP_
#define COMPILE_TYPE_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"



struct integer_type
{
    unsigned long bit_width;
};

typedef boost::variant<integer_type> type_kind;

struct type_info
{
    const symbol& node;
    type_kind kind;
    llvm::Type* llvm_type;
};

type_info compile_type(const symbol& type_node, llvm::LLVMContext& llvm_context);

#endif

