#include "macro_execution.hpp"
#include "error/macro_execution_error.hpp"

#include <llvm/IR/DerivedTypes.h>

#include <setjmp.h>
#include <limits>
#include <cstdint>
#include <stddef.h>

using llvm::LLVMContext;
using llvm::Type;
using llvm::IntegerType;
using llvm::PointerType;

using std::pair;
using std::vector;
using std::size_t;
using std::unique_ptr;
using std::move;
using std::int8_t;
using std::string;
using std::numeric_limits;

using boost::get;
using boost::blank;

using namespace macro_execution_error;

pair<node&, dynamic_graph> exeucte_macro(macro_function* func, const node& arg);


Type& llvm_node_type(LLVMContext& context)
{
    return *PointerType::getUnqual(IntegerType::get(context, 8));
}

struct execution_data_t
{
    jmp_buf jmp_env;
    dynamic_graph graph;
};

thread_local vector<execution_data_t> execution_data;

template<class T>
bool is_type(void* node_ptr)
{
    return get<T>(reinterpret_cast<dynamic_graph::node_data*>(node_ptr)) != nullptr;
}

node* as_node(node_ptr ptr)
{
    return reinterpret_cast<node*>(ptr);
}
node_ptr as_node_ptr(node* ptr)
{
    return reinterpret_cast<node_ptr>(ptr);
}


template<class NodeType>
struct get_data_impl;

template<>
struct get_data_impl<id_node>
{
    static dynamic_graph::id_data& get_data(node_ptr ptr)
    {
        return *reinterpret_cast<id_node*>(ptr);
    }
};
template<>
struct get_data_impl<lit_node>
{
    static dynamic_graph::lit_data& get_data(node_ptr ptr)
    {
        static_assert(offsetof(dynamic_graph::lit_data, first) == 0, "");
        return *reinterpret_cast<dynamic_graph::lit_data*>(ptr);
    }
};
template<>
struct get_data_impl<ref_node>
{
    static dynamic_graph::ref_data& get_data(node_ptr ptr)
    {
        static_assert(offsetof(dynamic_graph::ref_data, first) == 0, "");
        return *reinterpret_cast<dynamic_graph::ref_data*>(ptr);
    }
};
template<>
struct get_data_impl<list_node>
{
    static dynamic_graph::list_data& get_data(node_ptr ptr)
    {
        static_assert(offsetof(dynamic_graph::list_data, first) == 0, "");
        return *reinterpret_cast<dynamic_graph::list_data*>(ptr);
    }
};
template<>
struct get_data_impl<macro_node>
{
    static macro_node& get_data(node_ptr ptr)
    {
        return *reinterpret_cast<macro_node*>(ptr);
    }
};

template<class NodeType>
decltype(auto) get_data(node_ptr ptr)
{
    if(!as_node(ptr)->is<NodeType>())
        longjmp(execution_data.back().jmp_env, id("invalid_node_type"));
    return get_data_impl<NodeType>::get_data(ptr);
}

extern "C"
{

bool macro_is_id(node_ptr n)
{
    return as_node(n)->is<id_node>();
}
bool macro_is_lit(node_ptr n)
{
    return as_node(n)->is<lit_node>();
}
bool macro_is_ref(node_ptr n)
{
    return as_node(n)->is<ref_node>();
}
bool macro_is_list(node_ptr n)
{
    return as_node(n)->is<list_node>();
}
bool macro_is_macro(node_ptr n)
{
    return as_node(n)->is<macro_node>();
}

node_ptr macro_lit_create()
{
    return as_node_ptr(&execution_data.back().graph.create_lit(""));
}
uint64_t macro_lit_size(node_ptr n)
{
    return get_data<lit_node>(n).second.size();
}
int8_t macro_lit_get(node_ptr n, uint64_t index) noexcept
{
    string& str = get_data<lit_node>(n).second;
    if(index >= str.size())
        longjmp(execution_data.back().jmp_env, id("index_out_of_bounds"));
    return str[index];
}
void macro_lit_set(node_ptr n, uint64_t index, int8_t c)
{
    string& str = get_data<lit_node>(n).second;
    if(index >= str.size())
        longjmp(execution_data.back().jmp_env, id("index_out_of_bounds"));
    str[index] = c;
}
void macro_lit_push(node_ptr n, int8_t c)
{
    auto& lit_data = get_data<lit_node>(n);
    lit_data.second.push_back(c);
    char* str_begin = &lit_data.second[0];
    char* str_end = str_begin + lit_data.second.size();
    lit_data.first = lit_node{str_begin, str_end};
}
void macro_lit_pop(node_ptr n)
{
    auto& lit_data = get_data<lit_node>(n);
    lit_data.second.pop_back();
    char* str_begin = &lit_data.second[0];
    char* str_end = str_begin + lit_data.second.size();
    lit_data.first = lit_node{str_begin, str_end};
}

node_ptr macro_list_create()
{
    return as_node_ptr(&execution_data.back().graph.create_list({}));
}
uint64_t macro_list_size(node_ptr n)
{
    return get_data<list_node>(n).second.size();
}
node_ptr macro_list_get(node_ptr n, uint64_t index) noexcept
{
    vector<node*> children = get_data<list_node>(n).second;
    if(index >= children.size())
        longjmp(execution_data.back().jmp_env, id("index_out_of_bounds"));
    return as_node_ptr(children[index]);
}
void macro_list_set(node_ptr n, uint64_t index, node_ptr to_set)
{
    vector<node*>& children = get_data<list_node>(n).second;
    if(index >= children.size())
        longjmp(execution_data.back().jmp_env, id("index_out_of_bounds"));
    children[index] = as_node(to_set);
}
void macro_list_push(node_ptr n, node_ptr to_push)
{
    auto& list_data = get_data<list_node>(n);
    list_data.second.push_back(as_node(to_push));
    node** children_begin = &list_data.second[0];
    node** children_end = children_begin + list_data.second.size();
    list_data.first = list_node{children_begin, children_end};
}
void macro_list_pop(node_ptr n)
{
    auto& list_data = get_data<list_node>(n);
    list_data.second.pop_back();
    node** children_begin = &list_data.second[0];
    node** children_end = children_begin + list_data.second.size();
    list_data.first = list_node{children_begin, children_end};
}

node_ptr macro_ref_create()
{
    return as_node_ptr(&execution_data.back().graph.create_ref(""));
}
node_ptr macro_ref_get_identifier(node_ptr n)
{
    auto& ref_data = get_data<ref_node>(n);
    lit_node& lit = execution_data.back().graph.create_lit(save<string>(ref_data.first.identifier()));
    return as_node_ptr(&lit);
}
void macro_ref_set_identifier(node_ptr ref, node_ptr lit)
{
    auto& ref_data = get_data<ref_node>(ref);
    auto& lit_data = get_data<lit_node>(lit);
    ref_data.second = save<string>(lit_data.first);
    char* str_begin = &ref_data.second[0];
    char* str_end = str_begin + ref_data.second.size();
    ref_data.first = ref_node{str_begin, str_end, const_cast<node*>(ref_data.first.refered())};
}
bool macro_ref_has_refered(node_ptr ref)
{
    auto& ref_data = get_data<ref_node>(ref);
    return ref_data.first.refered() != nullptr;
}
node_ptr macro_ref_get_refered(node_ptr ref)
{
    auto& ref_data = get_data<ref_node>(ref);
    if(ref_data.first.refered() == nullptr)
        longjmp(execution_data.back().jmp_env, id("ref_null"));
    return as_node_ptr(const_cast<node*>(ref_data.first.refered()));
}
void macro_ref_set_refered(node_ptr ref, node_ptr new_refered)
{
    auto& ref_data = get_data<ref_node>(ref);
    ref_data.first.refered(as_node(new_refered));
}

node_ptr macro_to_node(size_t ptr_as_int)
{
    node* ptr = reinterpret_cast<node*>(ptr_as_int);
    return as_node_ptr(&execution_data.back().graph.add(*ptr));
}

node_ptr macro_call_macro(size_t ptr_as_int, node_ptr macro_arg)
{
    const macro_node& macro = *reinterpret_cast<macro_node*>(ptr_as_int);
    const list_node& arg = as_node(macro_arg)->cast_else<list_node>([&]
    {
        longjmp(execution_data.back().jmp_env, id("call_macro_arg_not_a_list"));
    });
    
    auto p = macro(rangeify(arg));
    node& result = p.first;
    execution_data.back().graph.add(move(p.second));
    return as_node_ptr(&result);
}

}


pair<node&, dynamic_graph> execute_macro(macro_function* func, const node& arg)
{
    auto p = dynamic_graph::clone(arg);
    execution_data.emplace_back();
    execution_data.back().graph = move(p.second);
    node& macro_arg = p.first;

    if(int error_id = setjmp(execution_data.back().jmp_env))
    {
        execution_data.pop_back();
        throw compile_exception(error_kind::MACRO_EXECUTION, error_id, blank());
    }

    node* result = as_node(func(as_node_ptr(&macro_arg)));

    dynamic_graph graph = move(execution_data.back().graph);
    execution_data.pop_back();
    return {*result, move(graph)};
}

pair<node&, dynamic_graph> execute_macro(macro_function* func, node_range args)
{
    auto node_pointers = save<vector<node*>>(mapped(args,
    [&](node& n) -> node*
    {
        return &n;
    }));
    node** children_begin = &node_pointers[0];
    node** children_end = children_begin + node_pointers.size();
    list_node top_level_list{children_begin, children_end};

    return execute_macro(func, top_level_list);
}

