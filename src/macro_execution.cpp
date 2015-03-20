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



Type& llvm_node_type(LLVMContext& context)
{
    return *PointerType::getUnqual(IntegerType::get(context, 8));
}

thread_local struct 
{
    jmp_buf jmp_env;
    dynamic_graph graph;
} execution_data;

/*
void check_valid_index(symbol_index si)
{
    if(si == numeric_limits<symbol_index>::max() || si >= execution_data.graph.data.size())
        longjmp(execution_data.jmp_env, id("invalid_node_index"));
}
*/

template<class T>
bool is_type(void* node_ptr)
{
    return get<T>(reinterpret_cast<dynamic_graph::node_data*>(node_ptr)) != nullptr;
}
/*
template<class T>
T& get_data(void* node_ptr)
{
    if(T* obj = get<T>(reinterpret_cast<dynamic_graph::node_data*>(node_ptr)))
        return *obj;
    else
        longjmp(execution_data.jmp_env, id("invalid_node_type"));
}
*/


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
        longjmp(execution_data.jmp_env, id("invalid_node_type"));
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
    return as_node_ptr(&execution_data.graph.create_lit(""));
}
uint64_t macro_lit_size(node_ptr n)
{
    return get_data<lit_node>(n).second.size();
}
int8_t macro_lit_get(node_ptr n, uint64_t index) noexcept
{
    string& str = get_data<lit_node>(n).second;
    if(index >= str.size())
        longjmp(execution_data.jmp_env, id("index_out_of_bounds"));
    return str[index];
}
void macro_lit_set(node_ptr n, uint64_t index, int8_t c)
{
    string& str = get_data<lit_node>(n).second;
    if(index >= str.size())
        longjmp(execution_data.jmp_env, id("index_out_of_bounds"));
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
    return as_node_ptr(&execution_data.graph.create_list({}));
}
uint64_t macro_list_size(node_ptr n)
{
    return get_data<list_node>(n).second.size();
}
node_ptr macro_list_get(node_ptr n, uint64_t index) noexcept
{
    vector<node*> children = get_data<list_node>(n).second;
    if(index >= children.size())
        longjmp(execution_data.jmp_env, id("index_out_of_bounds"));
    return as_node_ptr(children[index]);
}
void macro_list_set(node_ptr n, uint64_t index, node_ptr to_set)
{
    vector<node*>& children = get_data<list_node>(n).second;
    if(index >= children.size())
        longjmp(execution_data.jmp_env, id("index_out_of_bounds"));
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
    execution_data.graph = dynamic_graph{top_level_list};

    node_ptr macro_arg = visit<node_ptr>(*execution_data.graph.data.front(),
    [&](auto& v)
    {
        return reinterpret_cast<node_ptr>(&v);
    });

    if(int error_id = setjmp(execution_data.jmp_env))
        throw compile_exception(error_kind::MACRO_EXECUTION, error_id, blank());

    node_ptr result = func(macro_arg);
    node* n = reinterpret_cast<node*>(result);
    return {*n, move(execution_data.graph)};
}

