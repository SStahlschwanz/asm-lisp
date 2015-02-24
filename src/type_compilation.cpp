#include "type_compilation.hpp"

#include "error/type_compilation_exception.hpp"

#include <llvm/IR/DerivedTypes.h>

#include <string>
#include <algorithm>

using std::size_t;
using std::string;
using std::stoul;
using std::out_of_range;
using std::invalid_argument;
using std::distance;

using namespace symbol_shortcuts;

type_symbol compile_int(list_symbol::const_iterator begin, list_symbol::const_iterator end,
        compilation_context& context)
{
    if(distance(begin, end) != 1)
        throw int_invalid_argument_number{};

    const lit_symbol& bit_width_lit = begin->cast_else<lit_symbol>(
            int_invalid_argument_symbol{});

    unsigned long width;
    try
    {
        size_t index_after;
        string as_str{bit_width_lit.begin(), bit_width_lit.end()};
        width = stoul(as_str, &index_after);
        if(index_after != as_str.size())
            throw invalid_argument{""};
        if(width == 0 || width > 1024) // TODO: be less conservative
            throw out_of_range{""};
    }
    catch(const invalid_argument&)
    {
        throw int_invalid_argument_literal{};
    }
    catch(const out_of_range&)
    {
        throw int_out_of_range_bit_width{};
    }

    llvm::IntegerType* llvm_type = llvm::IntegerType::get(context.llvm(), width);
    return type{llvm_type};
}

type_symbol compile_struct(list_symbol::const_iterator begin, list_symbol::const_iterator end,
        compilation_context& context);
