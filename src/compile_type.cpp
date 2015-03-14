#include "compile_type.hpp"

#include "error/compile_type_error.hpp"
#include "core_unique_ids.hpp"
#include "core_utils.hpp"
#include "range.hpp"

#include <llvm/IR/DerivedTypes.h>

#include <boost/variant.hpp>

#include <string>
#include <algorithm>
#include <memory>

using std::size_t;
using std::string;
using std::stoul;
using std::out_of_range;
using std::invalid_argument;
using std::distance;
using std::string;
using std::vector;
using std::make_shared;

using boost::blank;

using llvm::IntegerType;
using llvm::LLVMContext;
using llvm::Type;
using llvm::PointerType;
using llvm::FunctionType;

using namespace compile_type_error;


unsigned long read_bit_width(const node& n)
{
    const lit_node& bit_width_lit = n.cast_else<lit_node>([&]()
    {
        fatal<id("int_invalid_argument_node")>(n.source());
    });

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
        fatal<id("int_invalid_argument_literal")>(bit_width_lit.source());
    }
    catch(const out_of_range&)
    {
        fatal<id("int_out_of_range_bit_width")>(bit_width_lit.source());
    }

    return width;
}

type_info compile_type(const node& type_node, LLVMContext& llvm_context)
{
    const node& resolved_node = resolve_refs(type_node);

    auto compile_from_range = [&](auto range) -> type_info
    {
        if(range.empty())
            fatal<id("empty_list_type")>(type_node.source());
        
        const node& resolved_constructor = resolve_refs(range.front());
        range.pop_front();

        const id_node& type_constructor = resolved_constructor.cast_else<id_node>([&]
        {
            fatal<id("invalid_type_constructor")>(resolved_constructor.source());
        });

        auto check_arity = [&](const char* type_constructor_name, unsigned int expected_argument_number)
        {
            if(expected_argument_number != length(range))
                fatal<id("invalid_type_constructor_argument_number")>(type_node.source());
        };
        
        switch(type_constructor.id())
        {
        case unique_ids::INT:
        {
            check_arity("int", 1);
            const node& bit_width_node = range.front();
            range.pop_front();

            unsigned long bit_width = read_bit_width(bit_width_node);
            Type* llvm_type = IntegerType::get(llvm_context, bit_width);
            assert(llvm_type);
            return {type_node, llvm_type, type_info::integer{bit_width}};
        }
        case unique_ids::PTR:
        {
            check_arity("ptr", 0);
            Type* llvm_type = PointerType::getUnqual(IntegerType::get(llvm_context, 8));
            assert(llvm_type);
            return {type_node, llvm_type, type_info::pointer{}};
        }
        case unique_ids::FUNCTION_SIGNATURE:
        {
            check_arity("function_signature", 2);
            const list_node& arg_types_list = range.front().template cast_else<list_node>([&]
            {
                fatal<id("invalid_argument_type_list")>(range.front().source());
            });
            range.pop_front();

            auto arg_types = save<vector<type_info>>(mapped(rangeify(arg_types_list), [&](const node& arg_type_node)
            {
                return compile_type(arg_type_node, llvm_context);
            }));
            auto llvm_args = save<vector<Type*>>(mapped(rangeify(arg_types), [&](const type_info& ti)
            {
                return ti.llvm_type;
            }));

            const node& return_type_node = range.front();
            range.pop_front();

            auto return_type = make_shared<type_info>(compile_type(return_type_node, llvm_context));

            FunctionType* llvm_type = FunctionType::get(return_type->llvm_type, llvm_args, false /* no vararg */);
            assert(llvm_type);
            return {type_node, llvm_type, type_info::function_signature{move(return_type), move(arg_types)}};
        }
        default:
            fatal<id("unknown_type_constructor")>(type_constructor.source());
        }
    };


    if(resolved_node.is<list_node>())
    {
        const list_node& list = resolved_node.cast<list_node>();
        return compile_from_range(rangeify(list));
    }

    return compile_from_range(static_range(resolved_node));
    /*
        if(type_node.empty())
            fatal<id("empty_list_type")>(type_node.source());
        const node& resolved_constructor = resolve_refs(type_node[0]);
        const id_node& type_constructor = resolved_constructor.cast_else<id_node>([&]
        {
            fatal<id("invalid_type_constructor")>(type_node[0].source());
        });

        auto check_arity = [&](const char* type_constructor_name, unsigned int expected_argument_number)
        {
            if(expected_argument_number != type_node.size() - 1)
                fatal<id("invalid_type_constructor_argument_number")>(type_node.source());
        };
        
        switch(type_constructor.id())
        {
        case unique_ids::INT:
        {
            check_arity("int", 1);
            const node& bit_width_node = type_node[1];
            unsigned long bit_width = read_bit_width(bit_width_node);
            Type* llvm_type = IntegerType::get(llvm_context, bit_width);
            return {node, llvm_type, type_info::integer{bit_width}};
        }
        case unique_ids::PTR:
        {
            check_arity("ptr", 0);
            Type* llvm_type = PointerType::getUnqual(IntegerType::get(llvm_context, 8));
            return {node, llvm_type, type_info::pointer{}};
        }
        case unique_ids::FUNCTION_SIGNATURE:
        {
            check_arity("function_signature", 2);
            const list_node& arg_types_list = type_node[1].cast_else<list_node>([&]
            {
                fatal<id("invalid_argument_type_list")>(type_node[1].source());
            });
            vector<type_info> arg_types;
            arg_types.reserve(arg_types_list.size());
            for(const node& arg_type_node : arg_types_list)
                arg_types.push_back(compile_type(arg_type_node, llvm_context));
            vector<Type*> llvm_args;
            llvm_args.reserve(arg_types.size());
            for(const type_info& ti : arg_types)
                llvm_args.push_back(ti.llvm_type);

            const node& return_type_node = type_node[2];
            auto return_type = make_shared<type_info>(compile_type(return_type_node, llvm_context));

            FunctionType* llvm_type = FunctionType::get(return_type->llvm_type, llvm_args, false /* no vararg );
            assert(llvm_type);
            return {node, llvm_type, type_info::function_signature{move(return_type), move(arg_types)}};
        }
        default:
            fatal<id("unknown_type_constructor")>(type_constructor.source());
        }
    }

    return compile_type(resolved_node, static_range(resolved_node), llvm_context);
    */
}

