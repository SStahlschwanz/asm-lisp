#ifndef COMPILE_TYPE_HPP_
#define COMPILE_TYPE_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"

#include <llvm/IR/Type.h>

struct type_info
{

    struct integer
    {
        unsigned long bit_width;
    };
    struct pointer
    {};
    struct function_signature
    {
        std::shared_ptr<type_info> return_type;
        std::vector<type_info> arg_types;
    };


    const symbol& node;
    llvm::Type* llvm_type;
    boost::variant
    <
        integer,
        pointer,
        function_signature
    > kind;
};

type_info compile_type(const symbol& type_node, llvm::LLVMContext& llvm_context);

#endif

